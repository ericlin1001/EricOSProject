#include "stdio.h"

void printInt(uint i,uint base){
	char buffer[10];
	int len=0;
	uint ch;
	if(i==0){
		printChar('0');
		return;
	}
	while(i!=0){
		ch=i%base;
		i/=base;
		if(ch<10){
			buffer[len++]=ch+'0';
		}else if(ch<16){
			buffer[len++]=ch-10+'A';
		}
	}
	for(len--;len>=0;len--){
		printChar(buffer[len]);
	}
}
void printTimesChar(char ch,int times){
	int i;
	for(i=0;i<times;i++)putChar(ch);
}
void printIntN(uint i,uint base,int n){
	/****bug: when base=2,n=16***/
	char buffer[10];
	int len=0;
	uint ch;
	if(i==0){
		printTimesChar('0',n);
	}else{
		while(i!=0){
			ch=i%base;
			i/=base;
			if(ch<10){
				buffer[len++]=ch+'0';
			}else if(ch<16){
				buffer[len++]=ch-10+'A';
			}
		}
		printTimesChar('0',n-len);
		if(len>n)len=n;
		for(len--;len>=0;len--){
			printChar(buffer[len]);
		}
	}
}
void CR(){
	printChar(0x0d);
	printChar(0x0a);
}
void printf(const char *str){
	while(*str){
		putChar(*str);
		str++;
	}
}
char getChar(){
	char ch;
	ch=(char)inputChar();
	if((char)ch==0x0d)printChar(0x0a);
	printChar(ch);
	return (char)ch;
}

void putChar(char ch){
	if(ch==0x0d)printChar(0x0a);
	if(ch==0x0a)printChar(0x0d);
	printChar((char)ch);
}
void printnf(const char *str,int size){
	while(size-- &&*str){
		putChar(*str++);
	}
}

void scanStr(char *buffer,int maxCount,char delim){
#define KEY_UP		 0x48
#define KEY_DOWN	 0x50
#define KEY_LEFT	 0x4b
#define KEY_RIGHT	 0x4d
#define KEY_DEL		 0x53
#define ASCII_BS 	 0x08
	int i=0;
	char ch=0;
	char keycode=0;
	int input;
	maxCount--;
	while(i<maxCount){
		input=inputChar();	
		ch=input;
		input>>=8;
		keycode=input;
		if(ch==delim)break;
		if(ch==0x0d){
			printChar(0x0d);
			printChar(0x0a);
			buffer[i]=ch;
			i++;
		}else if(ch==ASCII_BS){/*backspace*/
			if(i>0){
				i--;
				printChar(ASCII_BS);
				printChar(' ');
				printChar(ASCII_BS);
			}else{

			}
		}else if(ch==0){/*extensive key*/
			if(keycode==KEY_LEFT || keycode==KEY_DEL){
				if(i>0){
					i--;
					printChar(ASCII_BS);
					printChar(' ');
					printChar(ASCII_BS);
				}

			}else if(keycode==KEY_RIGHT){
			}else{
			}

		}else{
			if(isVisibleChar(ch)){
				printChar(ch);
				buffer[i]=ch;
				i++;
			}else{
			/*	if(isDebug){
					printChar('0');
					printChar('x');
					printInt(ch,16);
				}*/
				ch='?';
				printChar(ch);
				buffer[i]=ch;
				i++;
			}
		}
	}
	buffer[i]=0;
}