@echo off
set output=%~n1
set file1=%~n2
set file2=%~n3
set file3=%~n4
set file4=%~n5
set file5=%~n6
set file6=%~n7
set file7=%~n8
set file8=%~n9

tlink /3 /t %file1% %file2%  %file3% %file4% %file5% %file6% %file7% %file8%  ,%output%.bin,,

goto :exit

:exit

