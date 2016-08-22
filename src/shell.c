#include "stdio.h"
#include "stdio.h"
#include "StrFunc.h"
#include "process.h"
#include "filesys.h"
#include "memory.h"
#include "shell.h"


void jumpToShell(){
	pauseTimer();reset();shell();
}
int maxScheduleCount=3;
int scheduleCount=3;
extern int timerDelayCountMax;
BOOL isDebug=FALSE;
BOOL isRealRun=FALSE;
void scheduleAlgorithm(){
	PCB *pcb=getCurrentPCB();
	PCB *nextPCB=pcb->next;
	if(pcb!=nextPCB){
		if(pcb->status==PCB_EXIT){
#ifdef SHELL_DEBUG
			printf("\ntry to unload :");printf(pcb->name);CR();
#endif
			if(unloadProcess(pcb)){
#ifdef SHELL_DEBUG
				printf("unload the process successfully!\n");
#endif
			}else{
				printf("Error:can't unload the process !\n");
			}
		}
		if(strcmp(nextPCB->name,"idle.bin")==0){
			nextPCB=nextPCB->next;
		}
	}
	setCurrentPCB(nextPCB);
}

void monitorSchedule(){
	if(isRealRun){
		scheduleAlgorithm();	
	}else{
		if(scheduleCount-->0){
			/*printf(".");*/
			if(isDebug){
				printPCB(getCurrentPCB());
				printf("is paused.\n");
			}
			scheduleAlgorithm();	
		}else{
			scheduleCount=maxScheduleCount*800/timerDelayCountMax;
			if(isDebug){
				printf("\n*********stop scheduling***************!\n");
			}
			jumpToShell();
		}
	}
}


BOOL isRoot=FALSE;
char userName[50]="unknow";
#define ROOT_USERNAME "eric"
#define ROOT_PASSWORD "lin"
void login(){
	char usernameBuffer[50];
	char passwordBuffer[50];
	if(!isRoot){
		printf("username:");scanStr(usernameBuffer,50,(char)0x0d);CR();
		printf("password:");scanStr(passwordBuffer,50,(char)0x0d);CR();
		if(strcmp(usernameBuffer,ROOT_USERNAME)==0){
			if(strcmp(passwordBuffer,ROOT_PASSWORD)==0){
				isRoot=TRUE;
				strncpy(userName,ROOT_USERNAME,sizeof(ROOT_USERNAME));
				userName[sizeof(ROOT_USERNAME)]=0;
			}else{
				printf("wrong password.");
			}
		}else{
			printf("no user.");
		}
	}else{
		printf("You have been root user.");
	}
}

void shell(){
	char cmdBuffer[50];CR();
	while(1){
		printf(userName);
		if(isRoot){
			printf("#");
		}else{
			printf(">");
		}
		/*printf(currentDir);*/
		scanStr(cmdBuffer,100,(char)0x0d);CR();
		dealWithCMD(cmdBuffer);
	}
}


BOOL easyOpenPathFile(const char *path,FileHandler *pfh){
	/*path: /XXX/YYY/ZZZ   */
	char fileNameBuffer[12];
	const char *start=findChar(path,'/')+1;
	const char *end=strlen(path)+path;
	pfh->parent=NULL;/***from root directory****/
	while(start<end){
		strncpy(fileNameBuffer,start,11);
		*(char*)findChar(fileNameBuffer,'/')=0;
		if(!easyOpenFile(fileNameBuffer,pfh)){
			printf("Can't find ");printf(fileNameBuffer);
			return FALSE;
		}
		pfh->parent=pfh;
		start=findChar(start,'/')+1;
	}
	return TRUE;
}
FileHandler cd;
void initShell(){
	cd.parent=&cd;
	cd.firstCluster=0;
}
void printWorkingDir(){
	FileHandler curCD=cd;
	FileHandler parentCD=cd;
	char names[10][12];
	int i=0;
	/**take care: you should set parent,when you use easyOpenFile***/
	parentCD.parent=&parentCD;
	curCD.parent=&curCD;
	while(1){
		if(i>9)break;
		if(getCurrentDirName(names[i],&curCD)){
			names[i][11]=0;
			replaceConsecutive(trimRight(names[i],' '),' ','.');
			i++;
			if(easyOpenFile("..",&parentCD)){
				curCD=parentCD;
			}else{
				break;
			}
		}else{
			break;
		}

	}
	if(i==0)printf("/");
	i--;
	while(i>=0){
		printf("/");printf(names[i]);
		i--;
	}

}
void help(){
	static const char helpMess[]=
		"Help:\n"
		"login:                Login as root.(username:"ROOT_USERNAME" password:"ROOT_PASSWORD")\n"
		"1/start:              ContinueTimer\n"
		"launch <file>/<file>: launch a process loaded with <file>.\n"
		"                        (currectly <file> must in rootDir)\n"
		"realRun:              Actually run the OS, without entering\n"
		"                      this cmd line mode.\n"
		"2/printPCBs:          Show all process info.\n"
		"ls:                   list all files in current directory.\n"
		"cd <dir>:             change to a directory. \n"
		"pwd:                  print working directory.\n"
		"5/lsroot:             List all files in root directory.\n"
		"7 <num>:              set maxScheduleCount=<num>.\n"
		"8:                    switch to debug/nodebug mode.\n"
		"hello:                a hello message.\n"
		"help/? :              print this help message.\n"
		"showSector <num>:     show the content in <num>th sector .\n"
		"dump <seg> <offset> <num> :dump the memory in <seg>:<offset> with <num> bytes.\n"
		"                       (A very powerful tool for debugging,i use it a lot.)\n"
		"                      e.g. dump 0 0x7c00 512\n"
		"                      You will see the \"55 AA\" in the left-bottom corner.\n"
		"printmem              View the usage of memory.\n"
		"game                  A simple game.\n"
		"Please input command!\n"
		;
	printf(helpMess);
}
/*****************************SnackGame***********************/
typedef struct Node{
	unsigned char x,y;
	struct Node*next;
}Node;
typedef struct Dir{
	unsigned char dx,dy;
}Dir;
Node newNodePool[64];
int pNew=0;
Node *newNode(uchar tx,uchar ty){
	Node *p=&newNodePool[pNew++];
	p->x=tx;
	p->y=ty;
	return p  ;
}
Node *newNodeP(uchar tx,uchar ty,Node *parent){
	Node *p =newNode(tx,ty);
	p->next=parent;
	return p;
}

/*
   ;left,up,right ,down 1,2,3,4
   ;Y
   ;^
   ;|
   ;|
   ;0-------->X
   */
Dir dirs[5]={{0,0},{-1,0},{0,1},{1,0},{0,-1}};
Node *head,*food,*pivot;
unsigned char idir=3;
unsigned char len;
#define BASIC_SIZE 8
const int size=BASIC_SIZE;
uchar graph[BASIC_SIZE][BASIC_SIZE];
unsigned char winLen=24;
void initSnackGame(){
	head=newNode(2,4);
	head->next=head;
	head->next=newNodeP(1,4,head->next);
	head->next=newNodeP(0,4,head->next);
	len=3;
	food=newNode(0,0);
	;
	pivot=newNodeP(0,0,head->next);
}
BOOL isQuitSnackGame=FALSE;
void lose(){
	printf("lose\n");
	isQuitSnackGame=TRUE;
}
void win(){
	printf("win\n");
	isQuitSnackGame=TRUE;
}

int sseed=150;
uint rand(){
	sseed=sseed*sseed+1;
	return (uint)sseed;
}
void createGraph(){
	unsigned char i;
	unsigned char *pg=&graph[0][0];
	Node *p=head;
	for(i=0;i<size*size;i++)*pg++=0;
	do{
		graph[p->x][p->y]=1;
		p=p->next;
	}while(p!=head);
	graph[food->x][food->y]=2;
}
void move(unsigned char tdir){
	Dir dir;
	Node*p;
	if(tdir==0){
		tdir=idir;
	}
	dir=dirs[tdir];
	p=head;
	pivot->x=(head->x+dir.dx+size)%size;
	pivot->y=(head->y+dir.dy+size)%size;
	if(food->x==pivot->x && food->y==pivot->y){
		unsigned char index;
		unsigned char *pg;
		unsigned char ipg;
		/*eating food.*/
		/*  cout<<"eating food."<<endl;*/
		head=head->next=pivot;
		len++;
		if(len>=winLen)win();
		;
		pivot=newNodeP(0,0,head->next);                 
		/*place food to other places.*/

		index=rand()%(size*size-len);
		index++;
		createGraph();
		pg=&graph[0][0];
		ipg=0;
		while(index){if(*(pg+ipg++)==0)index--;};
		ipg--;

		food->x=ipg/size;
		food->y=ipg%size;

		return ;
	}else{
		p=head;
		while((p=p->next)!=head)if(p->x==pivot->x && p->y ==pivot->y)break;
		if(p->next==head){/*move back*/
			/*cout<<"Moving back"<<endl;*/
			tdir=idir;
			dir=dirs[tdir];
			pivot->x=(head->x+dir.dx)%size;
			pivot->y=(head->y+dir.dy)%size;
		}else if(p!=head){
			lose();
			return;
		}
		idir=tdir;
		/* cout<<"noramlly move"<<endl;*/
		/*noramlly move the snack.*/
		head=head->next=pivot;
		pivot=pivot->next;
		head->next=pivot->next;

	}

}
void print(){
	unsigned char i,j;
	createGraph();
	for(i=0;i<size;i++){
		for(j=0;j<size;j++){
			printInt(graph[j][size-1-i],10);
		}
		printf("\n");
	}
}
unsigned char convert(unsigned char ch){
	switch(ch){
		case 'a':
		case '1':
			/***left**/
			return 1;
		case 's':
		case '2':
			/***down**/
			return 4;
		case 'd':
		case '3':
			/****right****/
			return 3;
		case 'w':
		case '5':
			/*****up******/
			return 2;
	}
	return 0;
}
int snackGame(){
	unsigned char idir=0;
	printf("left:1\tup:5\tright:3\tdown:2\n");
	initSnackGame();
	while(!isQuitSnackGame){
		char inpuBuffer[2];
		/*system("cls");*/
		move(idir);
		print();

		printf("left:1/a  up:5/w  right:3/d  down:2/s\n");
		scanStr(inpuBuffer,2,(char)0x0d);CR();
		idir=inpuBuffer[0];
		idir=convert(idir);
	}
	return 0;
} 

/*********************end snackGame********************/

void ageGame(){
	int left=0,right=100;
	int middle;
	char an[10];
	while(left<right){
		middle=left+right;
		middle/=2;

		printf("\nYour age is ");
		printInt(middle,10);printf("?(Y/N)");
		scanStr(an,2,(char)0x0d);

		if(an[0]=='Y' || an[0]=='y' ){
			break;
		}

		printf("\nIs your age larger than ");
		printInt(middle,10);printf("?(Y/N)");
		scanStr(an,2,(char)0x0d);

		if(an[0]=='Y' || an[0]=='y' ){
			left=middle+1;
		}else{
			right=         middle-1;          
		}

	}
	printf("\nYeah!I have guessed out your age:");
	printInt(middle,10);
	printf("\n");
}
void game(){
	char input[2];
	printf("1.snackGame.\n2.ageGame.\nchoose:");
	scanStr(input,2,(char)0x0d);
	if(input[0]=='2'){
		ageGame();
	}else{
		snackGame();
	}
}


void shellTest(){
	newLoadClusterFileTo(3,0x4,0x1000,0);
}



MemBlock *pMemBlocks[10];
BOOL easyCMD(const char *cmd){
	if(cmd[0]=='0'){
		printf("this is shellTest()\n");
		shellTest();
	}else if(cmd[0]=='1'){
		continueTimer();
	}else if(matchReg(cmd,"printmem*")){
		printMemHole();
	}else if(cmd[0]=='2'){
		printAllPCB();
	}else if(matchReg(cmd,"pwd*")){
		printWorkingDir();
	}else if(strcmp(cmd,"realRun")==0){
		printf("now letting the OS really run....\n");
		isRealRun=TRUE;
		continueTimer();
#ifdef SHELL_DEBUG
	}else if(matchReg(cmd,"loadc *")){
		const char *p=cmd;
		int firstCluster;
		int fileSize;
		int loadSeg=0x1000;
		int loadOffset=0;
		FileHandler tcd;
		/***/
		p=findChar(p,' ')+1;
		firstCluster=parseInt(p);
		p=findChar(p,' ')+1;
		fileSize=parseInt(p);
		p=findChar(p,' ')+1;
		loadSeg=parseInt(p);
		p=findChar(p,' ')+1;
		loadOffset=parseInt(p);

		Trace(firstCluster);
		Trace(fileSize);
		Trace(loadSeg);
		Trace(loadOffset);

		tcd=cd;
		tcd.parent=&tcd;
		Trace(newLoadClusterFileTo(firstCluster,fileSize,loadSeg,loadOffset));
#endif
	}else if(matchReg(cmd,"getmem *")){
		int size=parseInt(findChar(findChar(cmd,' ')+1,' ')+1);
		int index=parseInt(findChar(cmd,' ')+1);

		Trace(index);Trace(size);
		pMemBlocks[index]=tryGetMemBlock(size);
	}else if(matchReg(cmd,"releasemem *")){
		int index=parseInt(findChar(cmd,' ')+1);
		releaseMemBlock(pMemBlocks[index]);
	}else if(matchReg(cmd,"game*")){
		game();
	}else if(matchReg(cmd,"cd *")){
		FileHandler tcd=cd;
		if(easyOpenFile(findChar(cmd,' ')+1,&tcd)){
			if(tcd.isDir){
				cd=tcd;
				printf("enter ");
				printFileHandler(&cd);
			}else{
				printf("It is not a directory!");
			}
		}else{
			printf("can't find path!\n");
		}

	}else if(matchReg(cmd,"open *")){
		FileHandler tcd=cd;
		tcd.parent=&tcd;
		if(easyOpenFile(findChar(cmd,' ')+1,&tcd)){
			if(tcd.isDir){
				cd=tcd;
				printf("enter ");
				printFileHandler(&cd);
			}else{
				printf("It is a file!\n");
				/*printFileHandler(&tcd);*/
				launchFileProcess(&tcd);
			}
		}else{
			printf("can't find file!\n");
		}
	}else if(cmd[0]=='5'){
		listPathFileEntry(NULL);
	}else if(cmd[0]=='7'){
		maxScheduleCount=parseInt(cmd+1);
		scheduleCount=maxScheduleCount*800/timerDelayCountMax;
		printf("set maxScheduleCount=");printInt(maxScheduleCount,10);
	}else if(cmd[0]=='8'){
		isDebug=!isDebug;
		if(isDebug){
			printf("Debug mode!");
		}else{
			printf("no Debug!");
		}
	}else{
		return FALSE;
	}
	return TRUE;
}

void dealWithCMD(const char *cmd){
#ifdef SHELL_DEBUG
	printf("cmd<");printf(cmd);printf(">\n");
#endif
	if(easyCMD(cmd)){
	}else if(strcmp(cmd,"printPCBs")==0){
		printAllPCB();
	}else if(strcmp(cmd,"login")==0){
		login();
	}else if(matchReg(cmd,"launch *")){
		FileHandler tcd=cd;
		tcd.parent=&tcd;
		if(easyOpenFile(findChar(cmd,' ')+1,&tcd)){
			if(tcd.isDir){
				cd=tcd;
				printf("enter ");
				printFileHandler(&cd);
			}else{
				printf("It is a file!\n");
				/*printFileHandler(&tcd);*/
				launchFileProcess(&tcd);
			}
		}else{
			printf("can't find file!\n");
		}
	}else  if(matchReg(cmd,"lsroot*")){
		listRootDirectory();
	}else if(matchReg(cmd,"ls*")){
		listPathFileEntry(&cd);
	}else if(matchReg(cmd,"showSector*")){
		int sectorNum=0;
		const char *p=cmd;
		p=findChar(p,' ')+1;
		sectorNum=parseInt(p);
		showSector(sectorNum);
	}else if(matchReg(cmd,"dump *")){
		int seg,offset,num;
		const char *p=cmd;
		p=findChar(p,' ')+1;
		seg=parseInt(p);
		p=findChar(p,' ')+1;
		offset=parseInt(p);
		p=findChar(p,' ')+1;
		num=parseInt(p);
		dumpMem(seg,offset,num);
	}else if(strcmp(cmd,"start")==0){
		continueTimer();
	}else if(cmd[0]=='?'){
		help();
	}else if(strcmp(cmd,"exit")==0){
		halt();
	}else if(strcmp(cmd,"hello")==0){
		printf("Hello, I'm ericOS,nice to meet you!");
	}else if(strcmp(cmd,"help")==0){
		help();
	}else{
		if(strcmp(cmd,"")!=0){
			FileHandler tcd=cd;
			tcd.parent=&tcd;
			if(easyOpenFile(cmd,&tcd)){
				if(tcd.isDir){
					printf("It can't run.\nIt is a directory!\n");
				}else{
					printf("It is a file!\n");
					/*	printFileHandler(&tcd);*/
					launchFileProcess(&tcd);
					printf("start run....\n");
					continueTimer();
				}
			}else{
				printf("Can't find cmd:");
				printf(cmd);
			}
		}

		else{
			printf("Can't find cmd:");
			printf(cmd);
		}
	}
	CR();
}



/**************basically function ****************/
