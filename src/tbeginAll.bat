@echo off
cls
echo Building,it may cost a few minutes.
echo please wait...
cmd /c "beginAll >tbeginAll_temp.txt"
type tbeginAll_temp.txt
echo .
echo .
echo **************Anaylse*******************
type tbeginAll_temp.txt|find /i "warn" 
type tbeginAll_temp.txt |find /i "can't"
type tbeginAll_temp.txt |find /i "undef"
type tbeginAll_temp.txt |find /i "err"
type tbeginAll_temp.txt |find /i "fail"
echo **************end Anaylse*******************
del /f tbeginAll_temp.txt
echo if no errors found,open builtBoot.img with VMware.
