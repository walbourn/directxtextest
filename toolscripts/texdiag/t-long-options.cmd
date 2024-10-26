@echo off
call startup.cmd %0

echo.
echo dumpbc -nologo %REFDIR%\LobbyCube.dds --target-x 10 --target-y 10
"%RUN%" dumpbc -nologo %REFDIR%\LobbyCube.dds --target-x 10 --target-y 10 > "%STDLOG%"
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo dumpbc -nologo %REFDIR%\LobbyCube.dds --target-x=10 --target-y=10
"%RUN%" dumpbc -nologo %REFDIR%\LobbyCube.dds --target-x=10 --target-y=10 >> "%STDLOG%"
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo dumpbc -nologo %REFDIR%\LobbyCube.dds --target-x:10 --target-y:10
"%RUN%" dumpbc -nologo %REFDIR%\LobbyCube.dds --target-x:10 --target-y:10 >> "%STDLOG%"
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
