#ifndef BASICTYPE_H
#define BASICTYPE_H
typedef unsigned char u8;	/*1×Ö½Ú*/
typedef unsigned int u16;	/*/2×Ö½Ú*/
typedef unsigned long u32;	/*/4×Ö½Ú*/
typedef unsigned int uint;
typedef int BOOL;
typedef unsigned char uchar;

#define HIGH16(m) (u16)(*((u16*)(&(m))+1))
#define LOW16(m) (u16)(m)
#define FALSE 0
#define TRUE 1
#define kernelSegment 0x9000
#define NULL 0

#define _DEBUG


#ifdef DEBUG
#define Trace(m) printf(#m"=");printInt(m,10);printf(" ");
#define TraceS(m) printf(#m"=");printf(m);printf(" ");
#define ASSERT(cond) if(!(cond)){printf("******Error:"#cond);}
#define DEBUG_MEM_MANAGMENT
#define SHELL_DEBUG
#define FILESYS_DEBUG
#else 
#define Trace(m)
#define TraceS(m)
#define ASSERT(cond)
#define _DEBUG_MEM_MANAGMENT
#define _SHELL_DEBUG
#define _FILESYS_DEBUG
#endif



#endif
