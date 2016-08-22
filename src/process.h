#ifndef PROCESS_H
#define PROCESS_H
#include "type.h"
#include "memory.h"
#include "filesys.h"
typedef struct RegisterImage{
int SS;
	int GS;
	int FS;
	int ES;
	int DS;
	int DI;
	int SI;
	int BP;
	int SP;
	int BX;
	int DX;
	int CX;
	int AX;
	int IP;
	int CS;
	int Flags;
}RegisterImage;

typedef enum PCB_Status{PCB_NORMAL,PCB_EXIT}PCB_Status;
typedef struct PCB{
	RegisterImage regImg;/***registers will be saved in this struct automactically by timer interrupt***/
	/******/
	int ID;
	PCB_Status status;
	struct PCB* next;
	struct PCB* prev;
	char name[16];
	MemBlock *pResideMemBlock;
}PCB;
/****import****/
void setCurrentPCB(PCB*pcb);
PCB* getCurrentPCB();

/*****my****/
PCB *newPCB();
void deletePCB(PCB *pcb);
void setupPCB(PCB *pcb,u16,u16);
void initPCBPool();

/***/

void PCBinit();


/********import***/
/*******timer*******/
void pauseTimer();
void continueTimer();/***this will continue the currentPCB**/

/**************/

extern u16 launchSeg;
extern int launchID;
BOOL launchProcess(const char *fileName);
BOOL launchFileProcess(FileHandler *pfh);
BOOL unloadProcess(PCB *pcb);

/****about process*********/
void printPCB(const PCB*pcb);
void printAllPCB();

#endif