@echo off
set name=%~n1
tasm %name%.asm %name%.obj  > asm_bat_temp.txt
type asm_bat_temp.txt |find "Error"
type asm_bat_temp.txt |find "Warn"
del /f asm_bat_temp.txt

