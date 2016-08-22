#include "type.h"
#include "stdio.h"
#include "StrFunc.h"
#include "memory.h"
#include "process.h"
#include "filesys.h"
#include "shell.h"
/*******import from asm***********/
void halt();
void reset();
/**************end import from asm****************/

/***export to asm****/
void cmain();
void actualSchdule();
/***end export to asm****/

/***********my function***********/
void cinit();
BOOL isShellMode;
/***********end my function***********/
void printVersion(){
	printf("ericOS 0.9      developed by Eric(email:463222898@qq.com).\n");
}
void cmain(){
	printVersion();
	isShellMode=TRUE;
	cinit();
	if(isShellMode){
		shell();
	}else{
		continueTimer();
	}
}

void cinit(){
	initReadSector();
	memManagerInit();
	PCBinit();
	initShell();
}

void actualSchdule(){
	if(isShellMode){
		monitorSchedule();
	}else{/**actually run....*/
		setCurrentPCB(getCurrentPCB()->next);
	}
}











