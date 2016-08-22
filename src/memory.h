#ifndef MEMORY_H
#define MEMORY_H
#include "type.h"

void memncpy(char *dest,const char *src,int size);
void memFarCpy(uint toSeg,uint toOffseg,uint fromSeg,uint fromOffset,uint size);
/****import from asm ***/
char getMem(int seg,int offset);
void setMem(int seg,int offset,u8 value);
/****/
void dumpMem(int seg,int offset,int num);

/*****************memory managment**********/
typedef u32 MemAddress;
#define memAddrSeg(addr) (u16)(addr>>4)
#define memAddrOffset(addr) (u16)(addr&0x0000000f)

typedef struct MemBlock{
#define MemBlockState_Fail 0
#define MemBlockState_Empty 1
#define MemBlockState_Used 2
	MemAddress start;
	MemAddress end;
   /*unsigned int size;*/
	/*int state;*/
}MemBlock;

typedef struct MemHole{
#define MemHoleState_Empty 0
#define MemHoleState_Partial 1   /****use for non-leaf node*/
#define MemHoleState_Full 2     /*****use for leaf node****/
	MemBlock mb;
	int logSize;
	int state;
	struct MemHole *leftChild;
	struct MemHole *rightChild;
	struct MemHole *fellow; /****its sibling node.***/
	struct MemHole *parent; /****its sibling node.***/
	struct MemHole *next;
}MemHole;
MemBlock* tryGetMemBlock(unsigned int size);
void releaseMemBlock(MemBlock * m);
void memManagerInit();
void printMemHole();
#endif