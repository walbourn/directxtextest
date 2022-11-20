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
echo v-cross-fnz -nologo -o %OUTTESTDIR%\vcrossfnz.bmp %REFDIR%\LobbyCube.dds
"%RUN%" v-cross-fnz -nologo -o %OUTTESTDIR%\vcrossfnz.bmp %REFDIR%\LobbyCube.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo h-tee -nologo -o %OUTTESTDIR%\htee.bmp %REFDIR%\LobbyCube.dds
"%RUN%" h-tee -nologo -o %OUTTESTDIR%\htee.bmp %REFDIR%\LobbyCube.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo h-strip -nologo -o %OUTTESTDIR%\hstrip.bmp %REFDIR%\LobbyCube.dds
"%RUN%" h-strip -nologo -o %OUTTESTDIR%\hstrip.bmp %REFDIR%\LobbyCube.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo v-strip -nologo -o %OUTTESTDIR%\vstrip.bmp %REFDIR%\LobbyCube.dds
"%RUN%" v-strip -nologo -o %OUTTESTDIR%\vstrip.bmp %REFDIR%\LobbyCube.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

REM *** Reverse commands ***

echo .
echo cube-from-hc -nologo -o %OUTTESTDIR%\cubehc.dds %OUTTESTDIR%\hcross.bmp
"%RUN%" cube-from-hc -nologo -o %OUTTESTDIR%\cubehc.dds %OUTTESTDIR%\hcross.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo cube-from-vc -nologo -o %OUTTESTDIR%\cubevc.dds %OUTTESTDIR%\vcross.bmp
"%RUN%" cube-from-vc -nologo -o %OUTTESTDIR%\cubevc.dds %OUTTESTDIR%\vcross.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo cube-from-vc-fnz -nologo -o %OUTTESTDIR%\cubevcfnz.dds %OUTTESTDIR%\vcrossfnz.bmp
"%RUN%" cube-from-vc-fnz -nologo -o %OUTTESTDIR%\cubevcfnz.dds %OUTTESTDIR%\vcrossfnz.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo cube-from-ht -nologo -o %OUTTESTDIR%\cubeht.dds %OUTTESTDIR%\htee.bmp
"%RUN%" cube-from-ht -nologo -o %OUTTESTDIR%\cubeht.dds %OUTTESTDIR%\htee.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo cube-from-hs -nologo -o %OUTTESTDIR%\cubehs.dds %OUTTESTDIR%\hstrip.bmp
"%RUN%" cube-from-hs -nologo -o %OUTTESTDIR%\cubehs.dds %OUTTESTDIR%\hstrip.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo .
echo cube-from-vs -nologo -o %OUTTESTDIR%\cubevs.dds %OUTTESTDIR%\vstrip.bmp
"%RUN%" cube-from-vs -nologo -o %OUTTESTDIR%\cubevs.dds %OUTTESTDIR%\vstrip.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
