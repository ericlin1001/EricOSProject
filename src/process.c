#include "process.h"
#include "stdio.h"


#define newPCBPool_Size 100
PCB newPCBPool[newPCBPool_Size];/*actually only newPCBPool_Size-1 can be used!*/
PCB * pEmptyPCBRoot;
void initPCBPool(){
	int i;
	pEmptyPCBRoot=newPCBPool;
	for(i=0;i<newPCBPool_Size;i++){
		newPCBPool[i].next=&newPCBPool[(i+1)%newPCBPool_Size];
	}
}
PCB *newPCB(){
	PCB *pivot=0;
	if(pEmptyPCBRoot->next!=pEmptyPCBRoot){
		pivot=pEmptyPCBRoot->next;
		pEmptyPCBRoot->next=pivot->next;
	}
	return pivot;
}
void deletePCB(PCB *pcb){
	if(pcb!=0){
		pcb->next=pEmptyPCBRoot->next;
		pEmptyPCBRoot->next=pcb;
	}
}

void setupRegisterImage(RegisterImage *pcb,u16 seg,u16 IP){
	pcb->GS=0xb800;
	pcb->FS=seg;
	pcb->ES=seg;
	pcb->DS=seg;
	/**/
	pcb->DI=0;
	pcb->SI=0;
	pcb->BP=0;
	pcb->SP=0x100-6;
	/***/
	pcb->BX=1;
	pcb->DX=3;
	pcb->CX=2;
	pcb->AX=0;
	/***/
	pcb->IP=IP;
	pcb->CS=seg;
	pcb->Flags=0x3202;
	pcb->SS=seg;

}

void setupPCB(PCB *pcb,u16 seg,u16 IP){
	setupRegisterImage(&pcb->regImg,seg,IP);
	pcb->ID=0;
	pcb->status=PCB_NORMAL;
	strcpy(pcb->name,"unnamed");
}

/***/
void printAllPCB();

BOOL initLaunchIdle(){
	FileHandler tcd;
	tcd.parent=NULL;
	if(easyOpenFile("idle.bin",&tcd)){
		if(tcd.isDir){
			printf("Error:Idle.bin should be a file,not a directory!!!\n");
		}else{
			if(launchFileProcess(&tcd)){
				return TRUE;
			}else{
				printf("Error:Idle.bin process should be created!!!\n");
			}
		}
	}else{
		printf("Error:Idle.bin is needed in image!!!\n");
	}
	return FALSE;
}

void PCBinit(){
	PCB*pcb=newPCB();
	initPCBPool();
	setupPCB(pcb,0x5000,0x100);
	pcb->next=pcb;
	pcb->prev=pcb;
	setCurrentPCB(pcb);
	if(initLaunchIdle()){
		detachPCB(getCurrentPCB());
		/*
		pcb=getCurrentPCB();
		setCurrentPCB(pcb->next);
		getCurrentPCB()->next=getCurrentPCB();
		deletePCB(pcb);*/
	}		
}


/***************/
u16 launchSeg=0x1000;
int launchID=1;

void attachPCB(PCB*pcb){
	pcb->next=getCurrentPCB()->next;
	getCurrentPCB()->next=pcb;
	/****/
	pcb->next->prev=pcb;
	pcb->prev=getCurrentPCB();
}
BOOL detachPCB(PCB*pcb){
	if(pcb->next!=pcb){
		if(getCurrentPCB()==pcb)setCurrentPCB(pcb->next);
		pcb->prev->next=pcb->next;
		pcb->next->prev=pcb->prev;
		return TRUE;
	}
	return FALSE;
}


BOOL launchFileProcess(FileHandler *pfh){
	PCB*pcb=newPCB();
	const uint stackSize=0x100;
	u16 loadSeg;
	u16 loadOffset;
	/*****/
	pcb->pResideMemBlock=tryGetMemBlock(pfh->fileSize+stackSize);
	if(pcb->pResideMemBlock==NULL){printf("launch process fail\n");deletePCB(pcb);return FALSE;}
	loadSeg=memAddrSeg(pcb->pResideMemBlock->start);
	loadOffset=memAddrOffset(pcb->pResideMemBlock->start)+stackSize;
	if(newLoadFileTo(pfh,loadSeg,loadOffset)){
		setupPCB(pcb,loadSeg,loadOffset);
		pcb->ID=launchID++;
		pcb->regImg.SP=pcb->regImg.IP-4;
		strcpy(pcb->name,pfh->fileName);
		attachPCB(pcb);
		/*printf("launch process success\n");*/
		return TRUE;
	}
	printf("launch process fail\n");
	releaseMemBlock(pcb->pResideMemBlock);
	deletePCB(pcb);
	return FALSE;
	/****/
}

void ProgramExitPort(){
PCB *pcb=getCurrentPCB();/***PCB is exiting...**/
pcb->status=PCB_EXIT;
printf(pcb->name);
printf(" is exiting...\n");
while(1);
}

BOOL launchProcess(const char *fileName){
	PCB*pcb=newPCB();
	PCB *currentPCB=getCurrentPCB();
	if(loadFileTo(fileName,launchSeg,0x100)){
		setupPCB(pcb,launchSeg,0x100);
		launchSeg+=0x1000;
		pcb->ID=launchID++;
		strcpy(pcb->name,fileName);
		replaceConsecutive(pcb->name,' ','.');
		pcb->next=currentPCB->next;
		currentPCB->next=pcb;
		printf("launch process success\n");
		return TRUE;
	}
	printf("launch process fail\n");
	return FALSE;
	/****/
}
BOOL unloadProcess(PCB *pcb){
	releaseMemBlock(pcb->pResideMemBlock);
	if(!detachPCB(pcb)){
		return FALSE;
	}
	deletePCB(pcb);
	return TRUE;
}


/****about process*********/
void printRegImg(RegisterImage *pcb){
printf("AX=");printIntN(pcb->AX,16,4); printf(" ");
	printf("BX=");printIntN(pcb->BX,16,4);printf(" ");
	printf("CX=");printIntN(pcb->CX,16,4);printf(" ");
	printf("DX=");printIntN(pcb->DX,16,4);printf(" ");
	/*CR();*/
	printf("CS=");printIntN(pcb->CS,16,4); printf(" ");
	printf("DS=");printIntN(pcb->DS,16,4);printf(" ");
	printf("ES=");printIntN(pcb->ES,16,4);printf(" ");
	printf("SS=");printIntN(pcb->SS,16,4);printf(" ");
	CR();
	printf("DI=");printIntN(pcb->DI,16,4); printf(" ");
	printf("SI=");printIntN(pcb->SI,16,4);printf(" ");
	printf("SP=");printIntN(pcb->SP,16,4);printf(" ");
	printf("BP=");printIntN(pcb->BP,16,4);printf(" ");
	/*CR();*/
	printf("GS=");printIntN(pcb->GS,16,4); printf(" ");
	printf("FS=");printIntN(pcb->FS,16,4);printf(" ");
	printf("IP=");printIntN(pcb->IP,16,4);printf(" ");
	printf("Flags=");printIntN(pcb->Flags,16,4);printf(" ");
	CR();
	
}

void printPCB(const PCB*pcb){
	printf("PID:");printInt(pcb->ID,10);printf(" ");
	printf("IM:");printf(pcb->name);printf(" ");
	printf("status:");printInt(pcb->status,10);printf(" ");
	printf("nextID:");printInt(pcb->next->ID,10);printf(" ");
	CR();
	/*****************/
	printRegImg(&pcb->regImg);
}

void printAllPCB(){
	PCB *currentPCB=getCurrentPCB();
	PCB *i=currentPCB;
	do{
		printPCB(i);CR();
		i=i->next;
	}while(i!=currentPCB);
}
