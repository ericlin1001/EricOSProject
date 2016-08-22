@echo off
rem **************begin*****************
call :deleteNoUseFile

rem pause
cmd /c "buildTestPro"
cmd /c "asm loader"
cmd /c "compile cloader"
cmd /c "compile StrFunc"
cmd /c "compile memory"
cmd /c "compile process"
cmd /c "compile filesys"
cmd /c "compile stdio"
cmd /c "compile shell"
cmd /c "buildbin loader  loader cloader strfunc memory process filesys stdio shell"
copy /y loader.bin builtImg\loader.bin
del loader.bin
cmd /c "buildImage"

call :deleteNoUseFile

rem **************end*************
goto :exit


:deleteNoUseFile:
set delFile=loader.obj
call :deleteFile

set delFile=cloader.obj
call :deleteFile

set delFile=loader.bin
rem call :deleteFile

set delFile=*.map
call :deleteFile

goto :eof

:deleteFile
if exist %delFile% del /f %delFile%
if exist %delFile% echo "Error: can't delete %delFile%,may be some applicatio is using it."
goto :eof


:exit

