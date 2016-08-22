#include "StrFunc.h"
int strncmp(const char *a,const char *b,int size){
	while(size--){
		if(*a!=*b)return (*a)-(*b);
		a++;
		b++;
	}
	return 0;
}
void strcpy( char *dest,const char *src){while(*src)*dest++=*src++;*dest=0;}
void strncpy( char *dest,const char *src,int size){while(size--)*dest++=*src++;}
int strlen(const char *str){
	int len=0;
	while(*str++)len++;
	return len;
}


/**************/
char* replaceConsecutive(char *src,char beReplaced,char newChar){
char *start=src;
	char *p=src;
	while(*src){
		if(*src==beReplaced){
			*p++=newChar;
			src++;
			while(*src &&*src==beReplaced)src++;
		}else{
			*p++=*src++;
		}
	}
	*p=0;
	return start;
}




int isDigital(char ch){
	return '0'<=ch && ch<='9';
}
int isUpperLetter(char ch){
	return 'A'<=ch && ch<='Z';
}
int isLowerLetter(char ch){
	return 'a'<=ch && ch<='z';
}
int isLetter(char ch){
	return isUpperLetter(ch)||isLowerLetter(ch);
}
void toLowerCase(char *str){
	str--;while(*++str)if(isUpperLetter(*str))*str+=0x20;
}
void toUpperCase(char *str){
	str--;while(*++str!=0)if(isLowerLetter(*str))*str-=0x20;
}
int parseInt(const char *src){
	int i=0;
	char buffer[30];
	char *str=buffer;
	strcpy(buffer,src);
	toUpperCase(buffer);
	if(*str=='0' && *(str+1)=='X'){
		/*printf("hex");CR();*/
		str+=2;
		while((isDigital(*str)|| ('A'<=*str &&*str<='F') )&& *str){
			i*=16;
			if(*str<='9'){
				i+=*str-'0';
			}else if(*str<='F'){
				i+=*str-'A'+10;
			}
			str++;
		}
	}else{
		while(isDigital(*str) && *str){
			i*=10;
			i+=*str-'0';
			str++;
		}
	}


	return i;
}

const char *findChar(const char *str,char find){while(*str&&*str!=find)str++;return str;}
int strcmp(const char *a,const char *b){
	while(*a && *b &&*a==*b){a++;b++;}
	return (*a)-(*b);
}
int isVisibleChar(char ch){
	return 0x20<=ch && ch<=0x7e;
}
int matchReg(const char *src,const char *reg){
	while(*src && *reg && *src==*reg && *reg!='*'){src++;reg++;}
	if(*reg=='*')return TRUE;
	if(*reg!=*src)return FALSE;
	return TRUE;
}
char * trimRight(char *str,char ch){
     char *end=strlen(str)+str-1;
     while(end>=str && *end==ch)end--;
     *(end+1)=0;
     return str;
}
char *trimLeft(char *str,char ch){
     char *end=strlen(str)+str-1;
     while(str<=end && *str==ch)str++;
     return str;     
}
char *trim(char *str,char ch){
return     trimLeft( trimRight(str,ch),ch);
     }
     
	 


BOOL intToBool(int i){
if(i==0)return FALSE;
return TRUE;
}