******************HOW TO START*******************
Method1:
1.double click startCmd.bat
2.input tbeginall<CR>
3.wait,until build ends.(Meanwhile you need to 
click <Yes> to continue the building)
4.open builtBoot.img with VMware.

(builtBoot.img is created directly from ./builtImg/
which means if you add sth to builtImg,it will appear
in builtBoot.img after built.)

Method2:
1.double click setup.bat
2.Open builtBoot.img with VMware.
*************************************************

************INTRODUCTION*****************
[TestProgram]:
a.asm,b.asm,c.asm,d.asm,idle.asm 

[OS]:
loader.asm,cloader.c,filesys.c,process.c,stdi.c,strfunc.c,memory.c,shell.c

[files detail]:
loader.asm: do some initalizing work,then jump to cloader::cmain()
cloader.c : the first part of c.In shellMode,it jump to shell.c::shell()
shell.c   : the most import function is shell.c::dealWithCMD(const char *cmd),
	    which is the main entry of all shell's command.
others... : find the function by yourselves.

[OS-function]:
memory.c  : implements a memory managment method:FellowSystem.
filesys.c : can read file,directory.
process.c : description of a process,can create a process,unload a process.
stdio.c   : some simple IO function,e.g. printf(const char *str),printInt(int i,int base)
shell.c   : compose some funcion offered by above files,to implement some command,
              such as scheduling,load a file as process.

[TestProgram-function]:
idle.asm	: <code:jmp $> it does nothing, just for idling.
a.asm 		: it will repeatedly print 'A' until a key is pressed,
       		  then it prints 'a' until another different key is 
       		  pressed,then exit.
b.asm 		: similar to a.asm,except it prints 'B' and 'b'.
c.asm 		: similar to a.asm,except it prints 'C' and 'c'.
d.asm 		: similar to a.asm,except it prints 'D' and 'd'.
*************************************************
