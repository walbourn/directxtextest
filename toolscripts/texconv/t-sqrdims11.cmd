@echo off
call startup.cmd %0

for /F "eol=#" %%r in (%REFDIR%\pow2.txt) do call :dosize %%r
goto :EOF

:dosize
if '%1' EQU '' goto :EOF
echo .
echo -nologo -w %1 -h %1 -f BC1_UNORM -ft DDS -sx _%1x%1 -o %OUTTESTDIR% %REFDIR%\1234-BMP.BMP 
"%RUN%" -nologo -w %1 -h %1 -f BC1_UNORM -ft DDS -sx _%1x%1 -o %OUTTESTDIR% %REFDIR%\1234-BMP.BMP 
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF

