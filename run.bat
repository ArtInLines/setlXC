@echo off

call comp.bat %1 %2
if exist %1.exe (
	call %1.exe
)