@echo off
set name=%~n1
tcc -mt -c -o%name%.obj %name%.c
