@echo off
call startup.cmd %0

echo.
echo -nologo -ft tiff -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
%RUN% -nologo -ft tiff -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --wic-quality 1.0 -ft tiff -o %OUTTESTDIR% --suffix _QONE %REFDIR%\1234-dds.dds
%RUN% -nologo --wic-quality 1.0 -ft tiff -o %OUTTESTDIR% --suffix _QONE %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --wic-uncompressed -ft tiff -o %OUTTESTDIR% --suffix _UNC %REFDIR%\1234-dds.dds
%RUN% -nologo --wic-uncompressed -ft tiff -o %OUTTESTDIR% --suffix _UNC %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --wic-quality 0.5 -ft jpeg -o %OUTTESTDIR% --suffix _Q5 %REFDIR%\1234-dds.dds
%RUN% -nologo --wic-quality 0.5 -ft jpeg -o %OUTTESTDIR% --suffix _Q5 %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --wic-quality 1.0 -ft jpeg -o %OUTTESTDIR% --suffix _QONE %REFDIR%\1234-dds.dds
%RUN% -nologo --wic-quality 1.0 -ft jpeg -o %OUTTESTDIR% --suffix _QONE %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -ft jxr -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
%RUN% -nologo -ft jxr -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --wic-lossless -ft jxr -o %OUTTESTDIR% --suffix _LOSSLESS %REFDIR%\1234-dds.dds
%RUN% -nologo --wic-lossless -ft jxr -o %OUTTESTDIR% --suffix _LOSSLESS %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
