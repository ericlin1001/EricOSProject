@echo off
rem echo ------README.TXT----------------------
start notepad README.TXT
rem type README.TXT
rem echo ------end README.TXT------------------
echo setup actually just run tbeginall.
if "%1"=="" set /p a=Are you sure to build?(press any key to begin)
cmd /c "setting & tbeginall"
del *.obj
echo end setup.
copy builtBoot.img ..\
set /p a=Press any key to exit...
exit
                                                  