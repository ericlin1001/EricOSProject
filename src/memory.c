#include "memory.h"
void memncpy(char *dest,const char *src,int size){
	while(size--)*dest++=*src++;
}
void memFarCpy(uint toSeg,uint toOffseg,uint fromSeg,uint fromOffset,uint size){
	while(size--){
		setMem(toSeg,toOffseg++,getMem(fromSeg,fromOffset++));
	}
}

void printAddress(int seg,int offset){
	printIntN(seg,16,4);printf(":");printIntN(offset,16,4);
}
void dumpMem(int seg,int offset,int num){
	int i;
	int offsetStart=(offset/16)*16;
	int offsetEnd=((offset+num-1)/16+1)*16;
	int offsetI=offsetStart;	
	CR();
	for(;offsetI<offsetEnd;offsetI++){
		if(offsetI%16==0){printAddress(seg,offsetI);printf("  ");}
		if(offsetI>=offset && offsetI<offset+num){
			printIntN(getMem(seg,offsetI),16,2);
			if(offsetI%16!=15 &&offsetI%8==7)printf("-");
			else printf(" ");
		}else{
			printTimesChar(' ',3);
		}
		if(offsetI%16==15)CR();
	}
}

/************************************/

/***************************Memory managment!!!********************************/


#define newMemHolePoolSize 128
const uint memManager_L=5;
const uint memManager_U=19;
const uint memManagerAlloMax=16; /***this should <=memManager_U***/
MemHole *rootHole;

MemHole newMemHolePool[newMemHolePoolSize];
MemHole *pEmptyMemHoleRoot; 

void printMemHole();
void initNewMemHolePool(){
	int i;
	for(i=0;i<newMemHolePoolSize-1;i++){
		newMemHolePool[i].next=&newMemHolePool[i+1];
	}
	newMemHolePool[newMemHolePoolSize-1].next=&newMemHolePool[0];
	pEmptyMemHoleRoot=&newMemHolePool[0];
}
MemHole *newMemHole(){
	MemHole *pmh=0;
	if(pEmptyMemHoleRoot->next!=pEmptyMemHoleRoot){/**root is not used**/
		pmh=pEmptyMemHoleRoot->next;
		pEmptyMemHoleRoot->next=pmh->next;
		pmh->next=0;
	}
	if(pmh!=0){
		pmh->leftChild=0;
		pmh->rightChild=0;
		pmh->fellow=0;
		pmh->parent=0;
	}
	return pmh;
}
void deleteMemHole(MemHole*pmh){
	if(pmh&&pmh->next==0){
		pmh->next=pEmptyMemHoleRoot->next;
		pEmptyMemHoleRoot->next=pmh;  
	}
}
void printMemHolePool(){
	MemHole *p;
	int count=0;
	for(p=pEmptyMemHoleRoot->next;p!=pEmptyMemHoleRoot;p=p->next){
		count++;
	}
	printf("available MemHolePool count:");printInt(count,10);CR();
}
/*********************/


void printMemBlock(MemBlock *pmb){
#ifdef DEBUG_MEM_MANAGMENT
#endif
    MemAddress addr=pmb->start;
    u16 offset;
    u16 seg;
	  
    offset =memAddrOffset(addr);
    seg =memAddrSeg(addr);
	
     printf(" 0x");printIntN(seg,16,4);printf(":");printIntN(offset,16,4);

     addr=pmb->end;
     offset =memAddrOffset(addr);
    seg =memAddrSeg(addr);
    
    printf(" 0x");printIntN(seg,16,4);printf(":");printIntN(offset,16,4);
   /* printf(" size:");printInt(pmb->end-pmb->start+1,10);CR();*/
	
}

void memManagerInit(){
	initNewMemHolePool();
	rootHole=newMemHole();/***use one holeNode**/
	rootHole->logSize=memManager_U;
	rootHole->state=MemHoleState_Empty;
	rootHole->mb.start=0x10000;
	rootHole->mb.end=  0x8FFFF;

				#ifdef DEBUG_MEM_MANAGMENT
	printf("managed memory range:");
	  printMemBlock(&rootHole->mb);
#endif
}


   void setupChildMemBlock(MemHole *parent){
     parent->leftChild->mb.start=parent->mb.start;
     parent->leftChild->mb.end=(parent->mb.start+parent->mb.end)>>1;
     parent->rightChild->mb.start=parent->leftChild->mb.end+1;
     parent->rightChild->mb.end=parent->mb.end;
}  
MemHole*getMemHoleR(MemHole *from,const int logi){                     
	MemHole* temp;
	if(from->logSize ==logi){
		if(from->state==MemHoleState_Empty){
			from->state=MemHoleState_Full;
			return from;
		}else{
			return 0;
		}
	}else if(from->logSize > logi){
		if(from->leftChild){/**this is same as if(from->rightChild)*/
			/****has child****/
			temp=getMemHoleR(from->leftChild,logi);
			if(temp)return temp;
			else return getMemHoleR(from->rightChild,logi);
		}else{/****no child,this is a leaf,split this one into two node***/
			if(from->state==MemHoleState_Empty){
				from->state=MemHoleState_Partial;
				/****/
				from->leftChild=newMemHole();
				if(from->leftChild==0){
				#ifdef DEBUG_MEM_MANAGMENT
					printf("no engouth pool ");
					#endif
					/***not enough talbe pool to descrip the memory***/
					return from;
				}
				from->rightChild=newMemHole();
				if(from->rightChild==0){
				#ifdef DEBUG_MEM_MANAGMENT
					printf("no engouth pool ");
					#endif
					/***not enough talbe pool to descrip the memory***/
					deleteMemHole(from->leftChild);  
					from->leftChild=0;
					return from;          
				}
				/****/
				temp=from->leftChild;
				temp->logSize=from->logSize-1;
				temp->state=MemHoleState_Empty;
				temp->parent=from;
				/****/
				temp=from->rightChild;
				temp->logSize=from->logSize-1;
				temp->state=MemHoleState_Empty;
				temp->parent=from;
				/***/
				from->leftChild->fellow=from->rightChild;
				from->rightChild->fellow=from->leftChild;
				/***/
				setupChildMemBlock(from);
				return getMemHoleR(from->leftChild,logi);
			}else{
				return 0;
			}
		}
	}else{
		return 0;/***fail****/
	}
}

MemHole*getMemHole(int logi){/***return 0 as fail***/
MemHole *pmh;
#ifdef DEBUG_MEM_MANAGMENT
	printf("try get :");printInt(logi,10);CR();
	#endif
	if(logi>memManagerAlloMax)return 0;
	/*	if(logi>memManager_U)return 0;*/
	if(logi<memManager_L)logi=memManager_L;
	pmh=getMemHoleR(rootHole,logi);
	#ifdef DEBUG_MEM_MANAGMENT
	if(pmh!=0){
               printf("allocate :");printInt(logi,10);printf(" success\n");
	}else{
          printf("allocate :");printInt(logi,10);printf(" fail\n");
	}
	
if(pmh)	printMemBlock(&pmh->mb);
	printMemHole (); 
	#endif
	return pmh;
}
void combineFellowHole(MemHole*pmh){/***input:ensure pmh->state ==MemHoleState_Empty***/
	if(pmh->fellow && pmh->fellow->state==MemHoleState_Empty){
		/***start combining...*/
		pmh=pmh->parent;
		deleteMemHole(pmh->leftChild);    
		pmh->leftChild=0;  
		deleteMemHole(pmh->rightChild);   
		pmh->rightChild=0;
		pmh->state=MemHoleState_Empty;
		combineFellowHole(pmh);         
	}
}
void releaseMemHole(MemHole*pmh){/****this should be a leaf node.***/
#ifdef DEBUG_MEM_MANAGMENT
	printf("release ");
	#endif
	if(pmh){
		printInt(pmh->logSize,10);printf(" .\n");
		pmh->state=MemHoleState_Empty;
		combineFellowHole(pmh);
	}
#ifdef DEBUG_MEM_MANAGMENT
	printMemHole();
	#endif
}



/***************memory interface**************/
MemBlock* tryGetMemBlock(unsigned int size){
	int i=1;
	MemHole *pmh;
	while(((unsigned int)((unsigned int)1<<i))<size)i++;
	#ifdef DEBUG_MEM_MANAGMENT
	printf("try get size:");printInt(size,10);CR();
	#endif
	pmh=getMemHole(i);
	if(pmh!=0){
		return &pmh->mb;
	}else{
		return 0;/***fail****/
	}
}
void releaseMemBlock(MemBlock * m){
	releaseMemHole((MemHole*)(m));
}
/****************end memory interface**************/


void visitNode(MemHole *pmh){
	/***pmh is ensured !=0***/
	if(pmh->leftChild){/***not a leaf node****/
	}else{
		/****is a leaf node***/
		int logi=pmh->logSize;
		int leftSpace=logi/2-1;
		char ch=' ';
		
		logi-=memManager_L-2;
		if(pmh->state==MemHoleState_Empty){
			ch=' ';
		}else if(pmh->state==MemHoleState_Partial){
			ch='.';
		}else if(pmh->state==MemHoleState_Full){
			ch='*';
		}
		
		printTimesChar(ch,leftSpace);
		if(pmh->logSize>9){
		}else{
            putChar(ch);
		}
		printInt(pmh->logSize,10);
		printTimesChar(ch,logi-leftSpace-2);
		printf("|");

	}
	#ifdef DEBUG_MEM_MANAGMENT
	#endif
}

void inorderTravser(MemHole *pmh){
	if(pmh!=0){
		if(pmh->leftChild){/***has child****/
			inorderTravser(pmh->leftChild);
			visitNode(pmh);
			inorderTravser(pmh->rightChild);
		}else{
			visitNode(pmh);
		}

	}
}
void printMemHole(){
	printf("begin\n|");
	inorderTravser(rootHole);
	CR();
	printMemHolePool();
	printf("end\n");
}
/***************************end Memory managment!!!********************************/
/*
void test(){
	MemBlock *pmh[10];
	int p=0;
	MemBlock *t;
	printMemHole();
	pmh[p++]=  tryGetMemBlock(4);
	pmh[p++]= tryGetMemBlock(128);
	pmh[p++]=  tryGetMemBlock(256);
	pmh[p++]= tryGetMemBlock(4056);
	pmh[p++]=  tryGetMemBlock(5000);
	releaseMemBlock(pmh[0]);
	releaseMemBlock(pmh[3]);
	releaseMemBlock(pmh[2]);
	releaseMemBlock(pmh[1]);
	pmh[p++]=  tryGetMemBlock(80000);
	releaseMemBlock(pmh[4]);
	t=pmh[p++]= tryGetMemBlock(90000);
	releaseMemBlock(t);
}

void test1(){
	MemHole *pmh[10];
	int p=0;
	MemHole *t;
	printMemHole();
	pmh[p++]=  getMemHole(2);
	pmh[p++]= getMemHole(7);
	pmh[p++]=  getMemHole(8);
	pmh[p++]= getMemHole(12);
	pmh[p++]=  getMemHole(18);
	releaseMemHole(pmh[0]);
	releaseMemHole(pmh[3]);
	releaseMemHole(pmh[2]);
	releaseMemHole(pmh[1]);
	pmh[p++]=  getMemHole(19);
	releaseMemHole(pmh[4]);
	t=pmh[p++]=  getMemHole(19);
	releaseMemHole(t);
}*/
/*
int main(){ 
	memManagerInit();
	test();
	getchar();
	return 0;
}
*/
