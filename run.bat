@echo off

call comp.bat %1 %2
if exist out.exe (
	call out.exe
)