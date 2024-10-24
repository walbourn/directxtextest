@echo off
call startup.cmd %0

echo.
echo -nologo --horizontal-flip -o %OUTTESTDIR% --prefix LO --suffix _HR %REFDIR%\1234-bmp.bmp
%RUN% -nologo --horizontal-flip -o %OUTTESTDIR% --prefix LO --suffix _HR %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --file-type HDR -o %OUTTESTDIR% --format R32G32B32_FLOAT --prefix LO_ %REFDIR%\1234-bmp.bmp
%RUN% -nologo --file-type HDR -o %OUTTESTDIR% --format R32G32B32_FLOAT --prefix LO_ %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --file-type HDR -o %OUTTESTDIR% --overwrite --format R16G16B16A16_FLOAT --prefix LO_ %REFDIR%\1234-bmp.bmp
%RUN% -nologo --file-type HDR -o %OUTTESTDIR% --overwrite --format R16G16B16A16_FLOAT --prefix LO_ %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --width 100 --height 50 -o %OUTTESTDIR% --suffix _100x50a %REFDIR%\1234-bmp.bmp
%RUN% -nologo --width 100 --height 50 -o %OUTTESTDIR% --suffix _100x50a %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --width=100 --height=50 -o %OUTTESTDIR% --suffix _100x50b %REFDIR%\1234-bmp.bmp
%RUN% -nologo --width=100 --height=50 -o %OUTTESTDIR% --suffix _100x50b %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --width:100 --height:50 -o %OUTTESTDIR% --suffix _100x50c %REFDIR%\1234-bmp.bmp
%RUN% -nologo --width:100 --height:50 -o %OUTTESTDIR% --suffix _100x50c %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
