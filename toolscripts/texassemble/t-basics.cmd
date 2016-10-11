@echo off
call startup.cmd %0

echo.
echo volume -nologo -o %OUTTESTDIR%\volume.dds %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg %REFDIR%\peppers.tiff
"%RUN%" volume -nologo -o %OUTTESTDIR%\volume.dds %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg %REFDIR%\peppers.tiff
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo volume -nologo -w 256 -h 256 -o %OUTTESTDIR%\volume256.dds %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg %REFDIR%\peppers.tiff
"%RUN%" volume -nologo -w 256 -h 256 -o %OUTTESTDIR%\volume256.dds %REFDIR%\lena.jpg %REFDIR%\fishingboat.jpg %REFDIR%\peppers.tiff
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo cube -nologo -o %OUTTESTDIR%\cubemap.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
"%RUN%" cube -nologo -o %OUTTESTDIR%\cubemap.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo cube -nologo -w 256 -h 256 -o %OUTTESTDIR%\cubemap256.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
"%RUN%" cube -nologo -w 256 -h 256 -o %OUTTESTDIR%\cubemap256.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo array -nologo -o %OUTTESTDIR%\array.dds %REFDIR%\lena.jpg %REFDIR%\peppers.tiff
"%RUN%" array -nologo -o %OUTTESTDIR%\array.dds %REFDIR%\lena.jpg %REFDIR%\peppers.tiff
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo array -nologo -w 256 -h 256 -o %OUTTESTDIR%\array256.dds %REFDIR%\lena.jpg %REFDIR%\peppers.tiff
"%RUN%" array -nologo -w 256 -h 256 -o %OUTTESTDIR%\array256.dds %REFDIR%\lena.jpg %REFDIR%\peppers.tiff
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo cubearray -nologo -o %OUTTESTDIR%\cubearray.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
"%RUN%" cubearray -nologo -o %OUTTESTDIR%\cubearray.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo cubearray -nologo -w 256 -h 256 -o %OUTTESTDIR%\cubearray256.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
"%RUN%" cubearray -nologo -w 256 -h 256 -o %OUTTESTDIR%\cubearray256.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
