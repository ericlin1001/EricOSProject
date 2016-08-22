.8086
_TEXT segment byte public 'CODE'
assume cs:_TEXT
DGROUP group _TEXT,_DATA,_BSS
org 100h
start:
;;;;;;;*********************begin***********************
jmp kernel


;**********************data*********************
SS_off equ 0*2
SP_off equ 8*2
currentProcessPCB dw 0

isTimerPause db 0
timerDelay equ 8
timerDelayCountMax dw timerDelay 
public _timerDelayCountMax
_timerDelayCountMax dw timerDelay
timerDelayCount dw 1; 计时器计数变量，初值=delay

kernelSegment equ 9000h
;*********************end data*****************

;************************macro************************
;Affect:ax,bl input:al
DisplayHex:
mov ah,0eh 	
mov bl,0 	
cmp al,9h
jbe DisplayHex_noDealWith
add al,7h
DisplayHex_noDealWith:
add al,30h
int 10h
ret

printChar macro char
push ax
push bx
mov ah,0eh
mov al,char
mov bl,0
int 10h
pop bx
pop ax
endm

;Affect:ax,cx,DisplayHex
_trace macro  reg
mov ch,reg
;
mov al,ch
mov cl,4
shr al,cl
call DisplayHex ;affect:ax,bl
;
mov al,ch
and al,01111b
call DisplayHex
endm

trace macro  reg
push ax
push bx
push cx
_trace reg
pop cx
pop bx
pop ax
endm

traceAX macro
trace ah
trace al
push ax
push bx
mov al,' '
mov bl,0
mov ah,0eh
int 10h
pop bx
pop ax
endm

CR macro ;Affect:ax,bl
mov ah,0eh 	
mov bl,0 
;
mov al,0ah
int 10h
mov al,0dh
int 10h
endm




initTimerVector:
	push es
	xor ax,ax
	mov es,ax			
	mov word ptr es:[20h],offset Timer	
	mov ax,cs 
	mov word ptr es:[22h],ax
	pop es
ret


initAllSegmentReg  macro 
mov ax,cs
mov ds,ax; DS = CS
mov es,ax; ES = CS
mov ss,ax; SS = cs
mov sp, 0ffffh; 
mov ax,0B800h; 
.386
mov gs,ax; GS = B800h
.8086
endm 

pauseTimer  macro 
mov byte ptr ds:[isTimerPause],1
endm

continueTimer  macro 
mov byte ptr ds:[isTimerPause],0
endm


EOI macro
; 发送中断处理结束消息给中断控制器
push ax
mov al,20h; AL = EOI
out 20h,al; 发送EOI到主8529A
out 0A0h,al; 发送EOI到从8529A
pop ax
endm

;****************end macro*********************


;*****************function*********************

;*******kernel*******

kernel:
initAllSegmentReg
pauseTimer
call clearScreen
call initAll

pauseTimer
call near ptr _cmain ;here to jmp into C...

jmp $; this should not be executed.
;*******end kernel*******

;registerSysCall(int sysCallNum,uint procSeg,uint procOffset);
registerSysCall:
ret

;makeSysCall(int sysCallNum)
initSysCall:
ret


initIntVector:
INT_NUM equ 21h
	push es
	xor ax,ax
	mov es,ax			
	mov word ptr es:[INT_NUM*4],offset INTProc
	mov word ptr es:[INT_NUM*4+2],cs
	pop es
ret

INTProc:
;now working process stack.
push ds
push es
sti

INT_start:
push ax
mov ax,cs
mov ds,ax
mov es,ax
pop ax

cmp ah,4ch
jz subint4ch
INT_end:

pop es
pop ds
iret

subint4ch:
call near ptr _ProgramExitPort
jmp INT_end


initAll:
sti
call initIntVector
call initSysCall
call initTimerVector
call SetTimer
ret

;****end initAll*******

; 启动下一进程函数
SetTimer:
;init the chip8253.
mov al,34h; 设控制字值
out 43h,al; 写控制字到控制字寄存器
mov ax,149 ; 每秒8000次中断,1193182/8000=149
out 40h,al; 写计数器0的低字节
mov al,ah; AL=AH
out 40h,al; 写计数器0的高字节
ret


Timer:
;push fl,push cs,push ip.
push ds
push ax
mov ax,kernelSegment; 为访问内核程序中的数据，须让
mov ds,ax; DS=kernelSegment
EOI
;
cmp byte ptr ds:[isTimerPause],0
jne tTimer_end
dec word ptr ds:[timerDelayCount]; 递减计数变量
jnz tTimer_end; >0：跳转到end处
mov ax ,word ptr ds:[timerDelayCountMax]
mov word ptr ds:[timerDelayCount],ax; =0：重置计数变量=初值delay
;***********

pauseTimer
pop ax
pop ds
jmp realTimer

tTimer_end: ; 中断处理善后
pop ax
pop ds
iret; 从中断处理返回


realTimer:
;**********start to save
;save register in process stack.
;push fl,push cx,push ip,have beend done automatically.
push ax
push cx
push dx
push bx

push sp;need to fix to correct sp.
push bp
push si
push di

.386
push ds
push es
push fs
push gs
push ss
.8086

mov ax,ss
mov ds,ax
mov si,sp
;assume es:kernelSegment
mov ax,cs
mov es,ax
mov di,word ptr es:[currentProcessPCB]
;
mov cx,16
cld
rep movsw;ds:si->es:di
add word ptr es:[di-16*2+SP_off],7*2
;
add sp,16*2;skip the fl,cs,ip.
;************end save

;now change to kernel mode!!!
initAllSegmentReg
call near ptr _actualSchdule
;bug,continueTimer maybe afftect the restoreCurrentProcess
continueTimer
jmp restoreCurrentProcess
;Actually,this instruction will not be exectued.

; ---------------------------------------------------------------


restoreCurrentProcess:
mov ax,cs
mov ds,ax
mov si,[currentProcessPCB]

;assume ds:kernelSegment
;setup the process stack.
mov ss,word ptr ds:[si+SS_off]
mov sp,word ptr ds:[si+SP_off]
sub sp,16*2

mov ax,ss
mov es,ax
mov di,sp

mov cx,16
cld
rep movsw;ds:si->es:di

.386
;start to restore
add sp,2; skip the ss
pop gs
pop fs
pop es
pop ds
.8086

pop di
pop si
pop bp
;pop sp,this should not be restored.
add sp,2;skip the sp.

pop bx
pop dx
pop cx
pop ax

iret


clearScreen:
mov ax, 600h; AH = 6,  AL = 0
mov bx, 700h; (BH = 7)backgroud is black,white word.
mov cx, 0; 左上角: (0, 0)
mov dx, 184fh; 右下角: (24, 79)
int 10h;  
;bit6~4 is background RGB
;bit3 is 1 foreground higtligh
;bit2~0 is foreground RGB
ret
_TEXT ends


;*****************import from C************
_TEXT segment byte public 'CODE'
extrn _cmain:near
extrn _printf:near
extrn _actualSchdule:near
extrn _ProgramExitPort:near
_TEXT ends
;*****************end import from C************


;*****************export to C*********************
_TEXT segment byte public 'CODE'
public SCOPY@
SCOPY@ proc far
arg_0 = dword ptr 6
arg_4 = dword ptr 0ah
push bp
mov bp,sp
push si
push di
push ds
lds si,[bp+arg_0]
les di,[bp+arg_4]
cld
shr cx,1
rep movsw
adc cx,cx
rep movsb
pop ds
pop di
pop si
pop bp
retf 8
SCOPY@ endp
;
public _printChar
_printChar proc
push bp
mov bp,sp
;***
mov al,[bp+4]
mov bl,0
mov ah,0eh
int 10h
;***
mov sp,bp
pop bp
ret
_printChar endp
;
public _inputChar
_inputChar proc
mov ah,0
int 16h
ret
_inputChar endp
;
public _pauseTimer
_pauseTimer:
pauseTimer
ret
;
public _continueTimer
_continueTimer:
continueTimer
jmp restoreCurrentProcess
ret
;
public _getCurrentPCB
_getCurrentPCB:
mov ax,word ptr [currentProcessPCB];
ret


public _setCurrentPCB
_setCurrentPCB:
push bp
mov bp,sp
mov ax,[bp+4]
mov word ptr [currentProcessPCB],ax
pop bp
ret


public _halt
_halt:
hlt
ret

public _reset
_reset:
pop bx
initAllSegmentReg
jmp bx
ret


;AL：扇区数(1~255)
;DL：驱动器号(0和1表示软盘，80H和81H等表示硬盘或U盘) 
;DH：磁头号(0~15)
;CH：柱面号的低8位
;CL：0~5位为起始扇区号(1~63)，6~7位为硬盘柱面号的高2位(总共10位柱面号，取值0~1023)
;ES : BX：读入数据在内存中的存储地址	返回值：
;	操作完成后ES : BX指向数据区域的起始地址
;	出错时置进位标志CF=1，错误代码存放在寄存器AH中
;	成功时CF=0、AL=0


;                     1               2                3               4            5                    6            7
;int readSector(u16 numSectors,u16 driverNum,u16 magneticNum,u16 cylinderNum,u16 startSectorNum,u16 loadSeg,u16 loadOffet);
;0 means OK.
;1 means error
public _readSector
_readSector:
push bp
mov bp,sp
;****
push es


;/****************/
mov al,[bp+2+4*2+1]
mov cl,6
shl al,cl
and al,011000000b
mov cl,[bp+2+5*2]
and cl,000111111b;
or cl,al
;
mov al,[bp+2+1*2]
mov dl,[bp+2+2*2]
mov dh,[bp+2+3*2]
mov ch,[bp+2+4*2]

mov es,[bp+2+6*2]
mov bx,[bp+2+7*2]
;call BIOS 13H,ah=02h to read sector
mov ah,02h
;traceAX
int 13h
;adc al,0
mov al,ah
;traceAX
pop es
;****
mov sp,bp
pop bp
ret

;char getMem(int seg,int offset);
public _getMem
_getMem:
push bp
mov bp,sp
;****
push es
mov es,[bp+2+1*2]
mov bx,[bp+2+2*2]
mov ax,es:[bx]
mov ah,0
pop es
;****
mov sp,bp
pop bp
ret


;void setMem(int seg,int offset,u8 value);
public _setMem
_setMem:
push bp
mov bp,sp
;****
push es
push ax

mov es,[bp+2+1*2]
mov bx,[bp+2+2*2]
mov al,[bp+2+3*2]
mov es:[bx],al

pop ax
pop es
;****
mov sp,bp
pop bp
ret

public LXURSH@
LXURSH@ proc far
cmp cl,10h
jnb short loc_10015
mov bx,dx
shr ax,cl
shr dx,cl
neg cl
add cl,10h
shl bx,cl
or ax,bx
retf
loc_10015:
sub cl,10h
mov ax,dx
xor dx,dx
shr ax,cl
retf
LXURSH@ endp

public LXMUL@
LXMUL@ proc far
push si
xchg ax,si
xchg ax,dx
test ax,ax
jz short loc_10009
mul bx
loc_10009:
xchg ax,cx
test ax,ax
jz short loc_10012
mul si
add cx,ax
loc_10012:
xchg ax,si
mul bx
add dx,cx
pop si
retf
LXMUL@ endp


;BOOL registerINT(int intNum,uint intProcSeg,uint intProcOffset);
public _registerInt
_registerInt:
push bp
mov bp,sp
;****
push es
mov es,[bp+2+1*2]
mov bx,[bp+2+2*2]
mov ax,es:[bx]
mov ah,0
pop es
;****
mov sp,bp
pop bp
ret

;BOOL softINT(int intNum)
public _softINT
_softINT:
push bp
mov bp,sp
;****
push es
mov es,[bp+2+1*2]
mov bx,[bp+2+2*2]
mov ax,es:[bx]
mov ah,0
pop es
;****
mov sp,bp
pop bp
ret


_TEXT ends
;*****************end export to C*********************

;************DATA segment*************
_DATA segment word public 'DATA'
_DATA ends
;*************BSS segment*************
_BSS	segment word public 'BSS'
_BSS ends
;**************end of file***********
end start

;/***************bugs:*************/
;bug,continueTimer maybe afftect the restoreCurrentProcess
;try to find a way,that pause the timer!!!!
;/*************end bugs****************/
