@echo off
if not exist obj mkdir obj
if not exist bin mkdir bin

SET COMMMON_FLAGS=-nologo -Zi -W4 -EHsc -Od -DDEBUG
rem SET COMMMON_FLAGS=-nologo -W4 -EHsc -O2

cl %COMMMON_FLAGS% src/win32_main.cpp -Febin/win32_WicWacWoe.exe -Ilib/ %OBJ_FILES% -Foobj/ User32.lib Gdi32.lib -link -SUBSYSTEM:WINDOWS -incremental:no -opt:ref
