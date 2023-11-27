@echo off
cmd /c if exist %1.c del %1.c
cd src
call setlX setlXC.stlx -p ../%1.stlx ../%1 %2
@REM Calling setlX turns the echo on again, so we need to turn it off again
@echo off
cd ..
if exist %1.c (
	call gcc -g -o %1 %1.c -I./deps/ -I.
)