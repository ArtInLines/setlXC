@echo off
cd src
call setlX setlXC.stlx -p ../%1.stlx %2
@REM Calling setlX turns the echo on again, so we need to turn it off again
@echo off
cd ..
call gcc -g -o out out.c -I./deps/