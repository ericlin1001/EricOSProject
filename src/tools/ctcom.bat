@echo off
set output=%~n1
cmd /c "asm %output%"
cmd /c "buildcom %output% %output%"
del %output%.obj
del %output%.map

goto :exit

:exit

