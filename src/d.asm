.8086
_TEXT segment byte public 'CODE'
assume cs:_TEXT
org 100h
start:jmp main

;*********main*************
main:
mov al,'D'
call printCharTilKey
mov al,'d'
call printCharTilKey
call exitPort
retf
;*********end main*************
exitPort:
mov ah,4ch
int 21h
ret

count dw 20
lastKey db 20
printCharTilKey:;input al as char
ShowChar: 
;***printAL***
mov ah,0eh 	
mov bl,0 		
int 10h
;****delay***
mov word ptr [count],100	
call delay
;***check key	
push ax		
call checkKey
cmp ax,0
pop ax
jz ShowChar			
ret


checkKey:;output ax
mov ah,1 			
int 16h 			
jnz hasKey		
;****
noKey:
xor ax,ax
ret
;*****
hasKey:
mov ah,0
int 16h
cmp [lastKey],al
jz noKey
mov [lastKey],al
mov ax,1
ret


;input:di
;2.please set ds properly
printStr:;1.Affect:ax,bx,di 
.386
pusha
mov ah,0eh 	
mov bl,0 	
printStr_Start:
mov al,[di]
cmp al,0
jz printStr_End
mov ah,0eh 	
mov bl,0 	
int 10h
inc di
jmp printStr_Start
printStr_End:
popa
.8086
ret

delay:;input count
dec word ptr [count]		; 
call littleDelay
cmp word ptr [count],0
jnz delay				; 
ret		

littleDelay:
mov cx,0
loop $
ret			


_TEXT ends
end start
