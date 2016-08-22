@echo off
set fromImg=boot5.img
set img=builtBoot.img
set imgDir=builtImg


rem *************making img*************
if exist %img% del %img% /f /q
if exist %img% (
echo *********Can't create %img%,some application is using it*********
) else (
copy %fromImg% %img% >nul
if not exist %img% (
echo *********Can't create %img%,may be %fromImg%  can't be found!*********
) else (
winimage %img% /i /y /h  %imgDir%
extract -l %img%
echo *********Build Image successfully!***********
)
)
