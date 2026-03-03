@echo off
call startup.cmd %0

echo.
echo from-mips -nologo -o %OUTTESTDIR%\mipmaps.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
"%RUN%" from-mips -nologo -o %OUTTESTDIR%\mipmaps.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo from-mips -nologo -o %OUTTESTDIR%\mipmaps2.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG %OUTTESTDIR%\mipmaps.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyzpos.JPG
"%RUN%" from-mips -nologo -o %OUTTESTDIR%\mipmaps2.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG %OUTTESTDIR%\mipmaps.dds %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyzpos.JPG
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo from-mips -nologo -o %OUTTESTDIR%\mipmaps3.dds -w 256 -h 256 -m 6 %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
"%RUN%" from-mips -nologo -o %OUTTESTDIR%\mipmaps3.dds -w 256 -h 256 -m 6 %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo from-mips -nologo -o %OUTTESTDIR%\mipmaps4.dds -w 32 -h 32 %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
"%RUN%" from-mips -nologo -o %OUTTESTDIR%\mipmaps4.dds -w 32 -h 32 %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
