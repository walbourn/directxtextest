@echo off
call startup.cmd %0

echo .
echo merge -nologo -f rgba -o %OUTTESTDIR%\merge.bmp %REFDIR%\fishingboat.jpg %REFDIR%\lena.jpg 
"%RUN%" merge -nologo -f rgba -o %OUTTESTDIR%\merge.bmp %REFDIR%\fishingboat.jpg %REFDIR%\lena.jpg 
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo merge -nologo -f rgba -o %OUTTESTDIR%\merge2.bmp --swizzle RGBg %REFDIR%\fishingboat.jpg %REFDIR%\lena.jpg 
"%RUN%" merge -nologo -f rgba -o %OUTTESTDIR%\merge2.bmp --swizzle RGBg %REFDIR%\fishingboat.jpg %REFDIR%\lena.jpg 
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo merge -nologo -f rgba -o %OUTTESTDIR%\merge3.bmp --swizzle GGGr %REFDIR%\fishingboat.jpg %REFDIR%\lena.jpg 
"%RUN%" merge -nologo -f rgba -o %OUTTESTDIR%\merge3.bmp --swizzle GGGr %REFDIR%\fishingboat.jpg %REFDIR%\lena.jpg 
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
