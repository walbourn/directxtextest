@echo off
call startup.cmd %0

echo.
echo dumpdds -o %OUTTESTDIR% -nologo %REFDIR%\LobbyCubeUnc.dds
"%RUN%" dumpdds -o %OUTTESTDIR% -nologo %REFDIR%\LobbyCubeUnc.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
