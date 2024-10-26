@echo off
call startup.cmd %0

echo.
echo volume -nologo --width 256 --height 256 -o %OUTTESTDIR%\volume256a.dds %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg %REFDIR%\peppers.tiff
"%RUN%" volume -nologo --width 256 --height 256 -o %OUTTESTDIR%\volume256a.dds %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg %REFDIR%\peppers.tiff
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo volume -nologo --width=256 --height=256 -o %OUTTESTDIR%\volume256b.dds %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg %REFDIR%\peppers.tiff
"%RUN%" volume -nologo --width=256 --height=256 -o %OUTTESTDIR%\volume256b.dds %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg %REFDIR%\peppers.tiff
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo volume -nologo --width:256 --height:256 -o %OUTTESTDIR%\volume256c.dds %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg %REFDIR%\peppers.tiff
"%RUN%" volume -nologo --width:256 --height:256 -o %OUTTESTDIR%\volume256c.dds %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg %REFDIR%\peppers.tiff
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
