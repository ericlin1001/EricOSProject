#ifndef STRFUNC_H
#define STRFUNC_H
#include "type.h"
int strncmp(const char *a,const char *b,int size);
void strcpy( char *dest,const char *src);
void strncpy( char *dest,const char *src,int size);
int strlen(const char *str);
/****/
char* replaceConsecutive(char *src,char beReplaced,char newChar);



/*************/

int isDigital(char ch);
int isUpperLetter(char ch);
int isLowerLetter(char ch);
int isLetter(char ch);
void toLowerCase(char *str);
void toUpperCase(char *str);
int parseInt(const char *src);
const char *findChar(const char *str,char find);
int strcmp(const char *a,const char *b);
int isVisibleChar(char ch);
int matchReg(const char *src,const char *reg);


/********/

char * trimRight(char *str,char ch);
char *trimLeft(char *str,char ch);
char *trim(char *str,char ch);

/***/
BOOL intToBool(int i);
#endif