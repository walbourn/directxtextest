@echo off
call startup.cmd %0

echo.
echo info -nologo %REFDIR%\lena.jpg
"%RUN%" info -nologo %REFDIR%\lena.jpg > "%STDLOG%"
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo analyze -nologo %REFDIR%\lena.jpg
"%RUN%" analyze -nologo %REFDIR%\lena.jpg >> "%STDLOG%"
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo compare -nologo %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg
"%RUN%" compare -nologo %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg >> "%STDLOG%"
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo diff -o %OUTDIR%\diff.jpg -nologo %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg
"%RUN%" diff -o %OUTDIR%\diff.jpg -nologo %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo dumpbc -nologo %REFDIR%\LobbyCube.dds
"%RUN%" dumpbc -nologo %REFDIR%\LobbyCube.dds -targetx 10 -targety 10 >> "%STDLOG%"
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo dumpdds -o %OUTDIR% -nologo %REFDIR%\LobbyCubeUnc.dds
"%RUN%" dumpdds -o %OUTDIR% -nologo %REFDIR%\LobbyCubeUnc.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
