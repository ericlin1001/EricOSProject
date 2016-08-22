#ifndef FILESYS_H
#define FILESYS_H
#include "type.h"

/*****File system*****/

typedef struct BPB {	
	/***offset by 11 in sector0****/
	u16  BPB_BytsPerSec;	
	u8   BPB_SecPerClus;	
	u16  BPB_RsvdSecCnt;	
	u8   BPB_NumFATs;	
	u16  BPB_RootEntCnt;	
	u16  BPB_TotSec16;
	u8   BPB_Media;
	u16  BPB_FATSz16;	
	u16  BPB_SecPerTrk;
	u16  BPB_NumHeads;
	u32  BPB_HiddSec;
	u32  BPB_TotSec32;	
}BPB;
typedef struct RootEntry {	
	char DIR_Name[11];
	u8   DIR_Attr;		
	char reserved[10];
	u16  DIR_WrtTime;
	u16  DIR_WrtDate;
	u16  DIR_FstClus;	/*fist cluster*/
	u32  DIR_FileSize;
}RootEntry;

extern BPB bpb;
extern u8 readSectorBuffer[512];
int readSector(u16 numSectors,u16 driverNum,u16 magneticNum,u16 cylinderNum,u16 startSectorNum,u16 loadSeg,u16 loadOffet);
void initReadSector();
int readSectorByLogic(int startSectors,int numSectors,u16 loadSeg,u16 loadOffset);
void readSectorToBuffer(int startSectors);
void readBytesToBuffer(int startBytes);
u8 *getSectorBuffer();
int loadFileTo(const char *fileName,int loadSeg,int loadOffset);
/****/

void listRootDirectory();
RootEntry * getFileEntry(const char *findFileName,RootEntry *pRootEntry);
BOOL existFile(const char *file);
u16 getFATValue(u16 index);
int clusterToSector(u16 cluster);
/***/
int loadFileTo(const char *fileName,int loadSeg,int loadOffset);
void printRootEntry(RootEntry *pRootEntry);
void showSector(int sectorNum);

/*****/
char *convertToFileName(char *const str);


/*********/
typedef struct FileHandler{
char fileName[12];/*XXX.EXT****/
BOOL isDir;/***DIR_Attr&0x10***/
u16  firstCluster;
u32  fileSize;
struct FileHandler *parent;/*the root->parent=NULL,parent->isDir=TRUE;**/
}FileHandler;

int listPathFileEntry(FileHandler*path);
BOOL fileOpen(FileHandler *openFileHandler,FileHandler *path);


BOOL easyOpenFile(const char *fileName,FileHandler *pfh);

void printFileHandler(FileHandler *fh);

int newLoadFileTo(FileHandler *fileHandler,int loadSeg,int loadOffset);
int newLoadClusterFileTo(u16 firstCluster,u32 fileSize,int loadSeg,int loadOffset);












/**************/

#endif