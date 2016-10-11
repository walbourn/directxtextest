@echo off
call startup.cmd %0

echo .
echo h-cross -nologo -o %OUTTESTDIR%\hcross.bmp %REFDIR%\LobbyCube.dds
"%RUN%" h-cross -nologo -o %OUTTESTDIR%\hcross.bmp %REFDIR%\LobbyCube.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo v-cross -nologo -o %OUTTESTDIR%\vcross.bmp %REFDIR%\LobbyCube.dds
"%RUN%" v-cross -nologo -o %OUTTESTDIR%\vcross.bmp %REFDIR%\LobbyCube.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo h-strip -nologo -o %OUTTESTDIR%\hstrip.bmp %REFDIR%\LobbyCube.dds
"%RUN%" h-strip -nologo -o %OUTTESTDIR%\hstrip.bmp %REFDIR%\LobbyCube.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo v-strip -nologo -o %OUTTESTDIR%\vstrip.bmp %REFDIR%\LobbyCube.dds
"%RUN%" v-strip -nologo -o %OUTTESTDIR%\vstrip.bmp %REFDIR%\LobbyCube.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
