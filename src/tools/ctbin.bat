@echo off
set output=%~n1
cmd /c "asm %output%"
cmd /c "buildbin %output% %output%"
del %output%.obj
del %output%.map

goto :exit

:exit

