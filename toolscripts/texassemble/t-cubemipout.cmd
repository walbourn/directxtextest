@echo off
call startup.cmd %0

set TWOMIPS=%REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG

echo.
echo cube-from-mips -nologo -o %OUTTESTDIR%\cubemips.dds %TWOMIPS%
"%RUN%" cube-from-mips -nologo -o %OUTTESTDIR%\cubemips.dds %TWOMIPS%
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

set FULLMIPS=%REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG
set FULLMIPS=%FULLMIPS% %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG
set FULLMIPS=%FULLMIPS% %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG
set FULLMIPS=%FULLMIPS% %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG
set FULLMIPS=%FULLMIPS% %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG
set FULLMIPS=%FULLMIPS% %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG

echo.
echo cube-from-mips -nologo -o %OUTTESTDIR%\cubemips2.dds %FULLMIPS%
"%RUN%" cube-from-mips -nologo -o %OUTTESTDIR%\cubemips2.dds %FULLMIPS%
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

set SIXMIPS=%REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG %REFDIR%\lobbyxpos.JPG
set SIXMIPS=%SIXMIPS% %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG %REFDIR%\lobbyxneg.JPG
set SIXMIPS=%SIXMIPS% %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG %REFDIR%\lobbyypos.JPG
set SIXMIPS=%SIXMIPS% %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG %REFDIR%\lobbyyneg.JPG
set SIXMIPS=%SIXMIPS% %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG %REFDIR%\lobbyzpos.JPG
set SIXMIPS=%SIXMIPS% %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG %REFDIR%\lobbyzneg.JPG

echo.
echo cube-from-mips -nologo -o %OUTTESTDIR%\cubemip3.dds -w 256 -h 256 -m 6 %SIXMIPS%
"%RUN%" cube-from-mips -nologo -o %OUTTESTDIR%\cubemip3.dds -w 256 -h 256 -m 6 %SIXMIPS%
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
