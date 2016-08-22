#ifndef STDIO_H
#define STDIO_H
#include "type.h"
/**************import from asm****************/
/*****IO***********/
int inputChar();
void printChar(char ch);
/************/
char getChar();
void putChar(char ch);
/*****/
void printInt(uint i,uint base);
void printIntN(uint i,uint base,int n);
void printTimesChar(char ch,int times);
void printf(const char *str);
void CR();
void printnf(const char *str,int size);

void scanStr(char *buffer,int maxCount,char delim);
#endif