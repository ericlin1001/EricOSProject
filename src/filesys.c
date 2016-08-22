#include "filesys.h"


BPB bpb;
u8 readSectorBuffer[512];

void initReadSector(){
	readSector(1,0,0,0,1,kernelSegment,(int)readSectorBuffer);
	memncpy((char*)&bpb,(char*)(readSectorBuffer+11),sizeof(BPB));
}

int readSectorFilter(u16 numSectors,u16 driverNum,u16 magneticNum,u16 cylinderNum,u16 startSectorNum,u16 loadSeg,u16 loadOffset){
	int res;
	char errorCode;
	/*
	   ;AL：扇区数(1~255)
	   ;DL：驱动器号(0和1表示软盘，80H和81H等表示硬盘或U盘) 
	   ;DH：磁头号(0~15)
	   ;CH：柱面号的低8位
	   ;CL：0~5位为起始扇区号(1~63)，6~7位为硬盘柱面号的高2位(总共10位柱面号，取值0~1023)
	   ;ES : BX：读入数据在内存中的存储地址	返回值：
	   ;?	操作完成后ES : BX指向数据区域的起始地址
	   ;?	出错时置进位标志CF=1，错误代码存放在寄存器AH中
	   ;?	成功时CF=0、AL=0
	   */
	ASSERT(1<=numSectors && numSectors<=255);
	ASSERT(0<=driverNum&& driverNum<=0x81);
	ASSERT(0<=magneticNum && magneticNum<=15);
	ASSERT(0<=cylinderNum && cylinderNum<=1023);

	/*
	   printf("trying to read sector!\n");

	   Trace(numSectors);Trace(driverNum);Trace(magneticNum);Trace(cylinderNum);Trace(startSectorNum);Trace(loadSeg);Trace(loadOffset);CR();

*/
	res=readSector( numSectors, driverNum, magneticNum, cylinderNum, startSectorNum, loadSeg, loadOffset);

	if((char)res==0){
		/*
		   printf("!!!!!!!!!!!!read sector successfully!!!!!!!!!!!!!!!\n");

		   Trace(numSectors);Trace(driverNum);Trace(magneticNum);Trace(cylinderNum);Trace(startSectorNum);Trace(loadSeg);Trace(loadOffset);CR();
		   */
	}else{
		/*
		   Trace(numSectors);Trace(driverNum);Trace(magneticNum);Trace(cylinderNum);Trace(startSectorNum);Trace(loadSeg);Trace(loadOffset);CR();
		   */
		printf("Error(");printInt(res,10);printf("):can't read sector!!******************\n");
	}
	return res;	
}

int readSectorByLogic(int startSectors,int numSectors,u16 loadSeg,u16 loadOffset){

#ifdef FILESYS_DEBUG
	printf("\nin readSectorByLogic:"); Trace(startSectors);Trace(numSectors);Trace(loadSeg);Trace(loadOffset);CR();
#endif

	return readSectorFilter(numSectors,0,(startSectors/bpb.BPB_SecPerTrk)%bpb.BPB_NumHeads,(startSectors/bpb.BPB_SecPerTrk)/bpb.BPB_NumHeads,startSectors%bpb.BPB_SecPerTrk+1,loadSeg,loadOffset);
}


void readSectorToBuffer(int startSectors){
#ifdef FILESYS_DEBUG
	printf("\nin readSectorToBuffer");
#endif
	readSectorByLogic(startSectors,1,kernelSegment,(u16)readSectorBuffer);
#ifdef FILESYS_DEBUG
	printf("exit readSectorToBuffer\n"); 
#endif
}

void readBytesToBuffer(int startBytes){
	readSectorToBuffer(startBytes/bpb.BPB_BytsPerSec);
}

u8 *getSectorBuffer(){
	return readSectorBuffer;
}
void listRootDirectory(){
	int i;
	RootEntry rootEntry;
	int startSector=bpb.BPB_RsvdSecCnt+bpb.BPB_HiddSec+bpb.BPB_NumFATs*(bpb.BPB_FATSz16!=0?bpb.BPB_FATSz16:bpb.BPB_TotSec32);
	char fileName[12];
	for(i=0;i<bpb.BPB_RootEntCnt;i++){
		if(i%(bpb.BPB_BytsPerSec/32)==0){readSectorToBuffer(startSector++);}
		memncpy((char*)&rootEntry,(char*)getSectorBuffer()+((i*32)%512),sizeof(RootEntry));
		if(rootEntry.DIR_Name[0]==0 || rootEntry.DIR_Name[0]==(char)0xe5){
			/***0 means can be used,0e5h means file has been deleted**/
		}else{
			if(intToBool(rootEntry.DIR_Attr&0x10)){printf("[");}
			printnf(rootEntry.DIR_Name,11);
			if(intToBool(rootEntry.DIR_Attr&0x10)){printf("]");}
			CR();
		}
	}
}
RootEntry * getFileEntry(const char *findFileName,RootEntry *pRootEntry){
	int i;
	int startSector=bpb.BPB_RsvdSecCnt+bpb.BPB_HiddSec+bpb.BPB_NumFATs*(bpb.BPB_FATSz16!=0?bpb.BPB_FATSz16:bpb.BPB_TotSec32);
	for(i=0;i<bpb.BPB_RootEntCnt;i++){
		if(i%(bpb.BPB_BytsPerSec/32)==0){readSectorToBuffer(startSector++);}
		memncpy((char*)pRootEntry,(char*)getSectorBuffer()+((i*32)%512),sizeof(RootEntry));
		if(pRootEntry->DIR_Name[0]==0 || pRootEntry->DIR_Name[0]==(char)0xe5){
			/***0 means can be used,0e5h means file has been deleted**/
		}else{
			if(strncmp(findFileName, pRootEntry->DIR_Name,11)==0)return pRootEntry;
		}
	}
	/***can't find the file***/
	return 0;
}
BOOL existFile(const char *file){
	RootEntry r;
	return (int)getFileEntry(file,&r);
}

u16 getFATValue(u16 index){
	u16 fat1StartSector=bpb.BPB_RsvdSecCnt;
	u16 fatValueOffset=index*3/2;
	u16 bytes;
	fat1StartSector+=fatValueOffset/bpb.BPB_BytsPerSec;
	fatValueOffset%=bpb.BPB_BytsPerSec;

#ifdef FILESYS_DEBUG
	printf("getFATValue test1\n");
#endif
	readSectorToBuffer(fat1StartSector);
#ifdef FILESYS_DEBUG
	printf("getFATValue test2\n");
#endif
	/****bug: dumpMem may incorrectly deals with the offset...***/
	/*******CR();*****/
	/*dumpMem(kernelSegment,(int)(getSectorBuffer()+fatValueOffset),32); */
	/*Trace(fatValueOffset);CR();*/
	bytes=*(u16*)(getSectorBuffer()+fatValueOffset);
	if(index%2==0){
		/***if index is even****/
#ifdef FILESYS_DEBUG
		printf("even\n");
#endif
		bytes&=0x0fff;
	}else{
#ifdef FILESYS_DEBUG
		printf("odd\n");
#endif

		bytes>>=4;
	}

#ifdef FILESYS_DEBUG
	printf("getFATValue test3\n");
	Trace(bytes);printf("=");printInt(bytes,16);CR();
#endif

	return bytes; 
}

int clusterToSector(u16 cluster){
	int startRootDirSector=bpb.BPB_RsvdSecCnt+bpb.BPB_HiddSec+bpb.BPB_NumFATs*(bpb.BPB_FATSz16!=0?bpb.BPB_FATSz16:bpb.BPB_TotSec32);
	int rootDirSize=bpb.BPB_RootEntCnt*32/bpb.BPB_BytsPerSec;
	return startRootDirSector+rootDirSize+(cluster-2)*bpb.BPB_SecPerClus;
}
int loadFileTo(const char *fileName,int loadSeg,int loadOffset){
	RootEntry fileEntry;
	u16 fileCluster=0;
	u16 fileNextCluster=0;
	if(getFileEntry(fileName,&fileEntry)!=0){
		printf("find file:");printf(fileEntry.DIR_Name);CR();
		/*printRootEntry(&fileEntry);*/
		fileCluster=fileEntry.DIR_FstClus;
		fileNextCluster=getFATValue(fileCluster);
		if((fileNextCluster&0xff8)==0xff0){/**this cluster is bad**/
			printf("Error:Disk around the file has been destoryed!");CR();
			return 0;
		}
		while((fileCluster&0xff8)!=0xff8){/***not end***/
			fileNextCluster=getFATValue(fileCluster);
			if((fileNextCluster&0xff8)==0xff0){/**this cluster is bad**/
				printf("Error:Disk around the file has been destoryed!");CR();
				return 0;
			}else{
				/*printf("cluster");printInt(clusterCount++,10);printf(":");
				  Trace(fileCluster);
				  Trace(clusterToSector(fileCluster));CR();
				  Trace(loadOffset);CR();*/
				readSectorByLogic(clusterToSector(fileCluster),1,loadSeg,loadOffset);
				/***actually this can be accelerated by read more than 1 sector***/
				loadOffset+=bpb.BPB_BytsPerSec;
				fileCluster=fileNextCluster;
			}
		}
	}else{
		printf("can't find file:");printf(fileName);CR();
		return 0;
	}
	return 1;
}


/***************/
#ifdef FILESYS_DEBUG
void printRootEntry(RootEntry *pRootEntry){
	TraceS(pRootEntry->DIR_Name);CR();
	Trace(pRootEntry->DIR_Attr);CR();
	Trace(pRootEntry->DIR_WrtTime);CR();
	Trace(pRootEntry->DIR_WrtDate);CR();
	Trace(pRootEntry->DIR_FstClus);CR();
	Trace(pRootEntry->DIR_FileSize);CR();
}
#endif


void showSector(int sectorNum){
	printf("In sector");printInt(sectorNum,10);printf(":\n");
	readSectorByLogic(sectorNum,1,kernelSegment,(int)&readSectorBuffer);
	dumpMem(kernelSegment,(int)&readSectorBuffer,160);
}
char *convertToFileName(char *const str){
	char *end,*dot,*i;
	str[11]=0;
	end=str+strlen(str)-1;
	dot=(char*)findChar(str,'.');
	i=str+10;

	if(dot==str){
		while(*i!='.'||i>end)*i--=' ';
		return str;                
	}
	while(end>dot)*i--=*end--;
	while(i>=dot)*i--=' ';
	toUpperCase(str);
	return str;
}


/*************************implement the path,you can list path/ ,or load path/file****************/
/**bug:we DO NOT distinguish the disk corrupte and the file end.**/
/*u16 getNextCluster(u16 clusterNO){
  return getFATValue(clusterNO);
  clusterNO=getFATValue(clusterNO);
  if((clusterNO&0xff8)==0xff0)clusterNO=0;
  if((clusterNO&0xff8)==0xff8)clusterNO=0;
  return clusterNO;
  }*/


RootEntry * getPathFileEntry(const char *findFileName,RootEntry *pRootEntry,FileHandler*path){
	int i;
	if(path==NULL || path->firstCluster==0x00){
		int startSector=bpb.BPB_RsvdSecCnt+bpb.BPB_HiddSec+bpb.BPB_NumFATs*(bpb.BPB_FATSz16!=0?bpb.BPB_FATSz16:bpb.BPB_TotSec32);
		for(i=0;i<bpb.BPB_RootEntCnt;i++){
			if(i%(bpb.BPB_BytsPerSec/32)==0){readSectorToBuffer(startSector++);}
			memncpy((char*)pRootEntry,(char*)getSectorBuffer()+((i*32)%512),sizeof(RootEntry));
			if(pRootEntry->DIR_Name[0]==0 || pRootEntry->DIR_Name[0]==(char)0xe5){
				/***0 means can be used,0e5h means file has been deleted**/
			}else{
#ifdef FILESYS_DEBUG
				printnf(pRootEntry->DIR_Name,11);printf("==");printnf(findFileName,11);printf("?");
#endif
				if(strncmp(findFileName, pRootEntry->DIR_Name,11)==0)return pRootEntry;
#ifdef FILESYS_DEBUG
				printf("not equ!\n");
#endif
			}
		}
		/***can't find the file***/
		return 0;
	}else{
		/******TODO:*****/
		u16 dirCluster=path->firstCluster;
		if(!path->isDir)return 0;
		while(dirCluster){
			readSectorToBuffer(clusterToSector(dirCluster));
			/*printf("cluster:");printInt(dirCluster,10);printf("\n");*/
			for(i=0;i<512;i+=32){

				memncpy((char*)pRootEntry,(char*)getSectorBuffer()+i,sizeof(RootEntry));
				if(pRootEntry->DIR_Name[0]==0 || pRootEntry->DIR_Name[0]==(char)0xe5){
					/***0 means can be used,0e5h means file has been deleted**/
				}else{
#ifdef FILESYS_DEBUG
					printnf(pRootEntry->DIR_Name,11);printf("==");printnf(findFileName,11);printf("?");
#endif
					if(strncmp(findFileName, pRootEntry->DIR_Name,11)==0)return pRootEntry;
#ifdef FILESYS_DEBUG
					printf("not equ!\n");
#endif
				}
			}
			dirCluster=getFATValue(dirCluster);
			if(dirCluster&0xff0 == 0xff0)break;
		}
		return 0;
	}
}


BOOL getCurrentDirName(char *name,FileHandler*path){
	u16 curretnCluster=path->firstCluster;
	int i;
	RootEntry re;
	RootEntry *pRootEntry=&re;

	if(!easyOpenFile("..",path))return FALSE;
	if(path==NULL || path->firstCluster==0x00){
		int startSector=bpb.BPB_RsvdSecCnt+bpb.BPB_HiddSec+bpb.BPB_NumFATs*(bpb.BPB_FATSz16!=0?bpb.BPB_FATSz16:bpb.BPB_TotSec32);
		for(i=0;i<bpb.BPB_RootEntCnt;i++){
			if(i%(bpb.BPB_BytsPerSec/32)==0){readSectorToBuffer(startSector++);}
			memncpy((char*)pRootEntry,(char*)getSectorBuffer()+((i*32)%512),sizeof(RootEntry));
			if(pRootEntry->DIR_Name[0]==0 || pRootEntry->DIR_Name[0]==(char)0xe5){
				/***0 means can be used,0e5h means file has been deleted**/
			}else{
				if(pRootEntry->DIR_FstClus==curretnCluster){
					strncpy(name,pRootEntry->DIR_Name,11);
					return TRUE;
				}
			}
		}
		/***can't find the file***/
		return FALSE;
	}else{
		/******TODO:*****/
		u16 dirCluster=path->firstCluster;
		if(!path->isDir)return 0;
		while(dirCluster){
			readSectorToBuffer(clusterToSector(dirCluster));
			/*printf("cluster:");printInt(dirCluster,10);printf("\n");*/
			for(i=0;i<512;i+=32){

				memncpy((char*)pRootEntry,(char*)getSectorBuffer()+i,sizeof(RootEntry));
				if(pRootEntry->DIR_Name[0]==0 || pRootEntry->DIR_Name[0]==(char)0xe5){
					/***0 means can be used,0e5h means file has been deleted**/
				}else{
					if(pRootEntry->DIR_FstClus==curretnCluster){
						strncpy(name,pRootEntry->DIR_Name,11);
						return TRUE;
					}
				}
			}
			dirCluster=getFATValue(dirCluster);
			if(dirCluster&0xff0 == 0xff0)break;
		}
		return FALSE;
	}
}

void printFileName(char *fileName){
	char nameBuffer[15];
	/*printnf(fileName,11);*/
	strncpy(nameBuffer,fileName,11);
	nameBuffer[11]=0;
	/*printf(nameBuffer);*/
	printf(replaceConsecutive(trim(nameBuffer,' '),' ','.'));
}

int listPathFileEntry(FileHandler*path){
	int i;
	RootEntry re;
	RootEntry *pRootEntry=&re;
	/******/
	char fileCount=0;

	if(path==NULL|| path->firstCluster==0x00){
		int startSector=bpb.BPB_RsvdSecCnt+bpb.BPB_HiddSec+bpb.BPB_NumFATs*(bpb.BPB_FATSz16!=0?bpb.BPB_FATSz16:bpb.BPB_TotSec32);
		for(i=0;i<bpb.BPB_RootEntCnt;i++){
			if(i%(bpb.BPB_BytsPerSec/32)==0){readSectorToBuffer(startSector++);}
			memncpy((char*)pRootEntry,(char*)getSectorBuffer()+((i*32)%512),sizeof(RootEntry));
			if(pRootEntry->DIR_Name[0]==0 || pRootEntry->DIR_Name[0]==(char)0xe5){
				/***0 means can be used,0e5h means file has been deleted**/
			}else{
				if(intToBool(pRootEntry->DIR_Attr&0x10)){printf("[");}
				printFileName(pRootEntry->DIR_Name);
				if(intToBool(pRootEntry->DIR_Attr&0x10)){printf("]");}
				printf("   ");
				fileCount++;
				if(fileCount%5==0)CR();
			}
		}
		/***can't find the file***/
		return 0;
	}else{
		/******TODO:*****/
		u16 dirCluster=path->firstCluster;
		int i;
		if(!path->isDir)return 0;
		while(dirCluster){
			readSectorToBuffer(clusterToSector(dirCluster));
			for(i=0;i<512;i+=32){
				memncpy((char*)pRootEntry,(char*)getSectorBuffer()+i,sizeof(RootEntry));
				if(pRootEntry->DIR_Name[0]==0 || pRootEntry->DIR_Name[0]==(char)0xe5){
					/***0 means can be used,0e5h means file has been deleted**/
				}else{
					if(intToBool(pRootEntry->DIR_Attr&0x10)){printf("[");}
					printFileName(pRootEntry->DIR_Name);
					if(intToBool(pRootEntry->DIR_Attr&0x10)){printf("]");}
					printf("   ");
					fileCount++;
					if(fileCount%5==0)CR();
				}
			}
			dirCluster=getFATValue(dirCluster);
			if(dirCluster&0xff0 == 0xff0)break;
		}
	}
	return 0;
}


BOOL fileOpen(FileHandler *openFileHandler,FileHandler *path){
	RootEntry re;
	char realFileName[12];
	strncpy(realFileName,openFileHandler->fileName,11);
	convertToFileName(realFileName);
	if(getPathFileEntry(realFileName,&re,path)){
		openFileHandler->isDir=intToBool(re.DIR_Attr);
		openFileHandler->firstCluster=re.DIR_FstClus;
		openFileHandler->fileSize=re.DIR_FileSize;
		openFileHandler->parent=path;
		return TRUE;
	}
	return FALSE;
}
BOOL easyOpenFile(const char *fileName,FileHandler *pfh){
	strncpy(pfh->fileName,fileName,11);
	return fileOpen(pfh,pfh->parent);
}

void printFileHandler(FileHandler *fh){
	printf("fileName: ");
	if(fh->isDir){printf("[");}
	printnf(fh->fileName,11);
	if(fh->isDir){printf("]");}
	printf(" size:0x");printInt(fh->fileSize,16);
	printf("\nfirstCluster:");printInt(fh->firstCluster,10);
	printf("\n");
}

int readSomeBytes(u16 sector,int toSeg,int toOffset,u32 numBytes){
	readSectorToBuffer(sector);
	/*Trace(numBytes);*/
	memFarCpy(toSeg,toOffset,kernelSegment,getSectorBuffer(),numBytes);
	return 1;

}	
int newLoadFileTo(FileHandler *fileHandler,int loadSeg,int loadOffset){
	u16 fileCluster=0;
	u16 fileNextCluster=0;
	u32 numBytes;
	/*u16 clusterCount=0;*/
	u32 readBytes=0;
	/******TODO:*****/
	fileCluster=fileHandler->firstCluster;
#ifdef FILESYS_DEBUG
	printf("start reading1....");
#endif
	fileNextCluster=getFATValue(fileCluster);
	if((fileNextCluster&0xff8)==0xff0){
		printf("Error:Disk around the file has been destoryed!");CR();
		return 0;
	}
	while((fileCluster&0xff8)!=0xff8){
		fileNextCluster=getFATValue(fileCluster);
		if((fileNextCluster&0xff8)==0xff0){
			printf("Error:Disk around the file has been destoryed!");CR();
			return 0;
		}else{
			if(readBytes+512>fileHandler->fileSize){
#ifdef FILESYS_DEBUG
				printf("\start reading the else bytes not enought one sector!\n");
#endif
				numBytes=fileHandler->fileSize-readBytes;
				readSomeBytes(clusterToSector(fileCluster),loadSeg,loadOffset,numBytes);
				return 1;       
			}
			readSectorByLogic(clusterToSector(fileCluster),1,loadSeg,loadOffset);
			readBytes+=512;
			loadOffset+=bpb.BPB_BytsPerSec;
			fileCluster=fileNextCluster;
		}
	}
	return 1;
}

int newLoadClusterFileTo(u16 firstCluster,u32 fileSize,int loadSeg,int loadOffset){
	u16 fileCluster=0;
	u16 fileNextCluster=0;
	u32 numBytes;
	u32 readBytes=0;
	/******TODO:*****/
	fileCluster=firstCluster;
#ifdef FILESYS_DEBUG
	printf("start reading1....");
#endif

	fileNextCluster=getFATValue(fileCluster);

	printf("start reading2....");


	if((fileNextCluster&0xff8)==0xff0){
		printf("Error1:Disk around the file has been destoryed!");CR();
		return 0;
	}
	while((fileCluster&0xff8)!=0xff8){
		fileNextCluster=getFATValue(fileCluster);
		if((fileNextCluster&0xff8)==0xff0){
			printf("Error2:Disk around the file has been destoryed!");CR();
			return 0;
		}else{
			if(readBytes+512>fileSize){
#ifdef FILESYS_DEBUG
				printf("\start reading the else bytes not enought one sector!\n");
#endif
				numBytes=fileSize-readBytes;
				readSomeBytes(clusterToSector(fileCluster),loadSeg,loadOffset,numBytes);
				return 1;       
			}
			readSectorByLogic(clusterToSector(fileCluster),1,loadSeg,loadOffset);
			readBytes+=512;
			loadOffset+=bpb.BPB_BytsPerSec;
			fileCluster=fileNextCluster;
		}
		printf("next...\n");
	}
	return 1;
}
