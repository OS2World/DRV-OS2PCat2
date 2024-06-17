@echo off
REM create a listing file of real mode assembly code so that we can pick up the instruction code bytes in main.cpp
alp.exe -Mb +Fl +Lm -Llp:0 -Lr -Lf +Ls -Fo realmode.asm
