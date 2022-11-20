@echo off
call startup.cmd %0

echo .
echo gif -nologo -o %OUTTESTDIR%\flame.dds %REFDIR%\flame.gif
"%RUN%" gif -nologo -o %OUTTESTDIR%\flame.dds %REFDIR%\flame.gif
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

