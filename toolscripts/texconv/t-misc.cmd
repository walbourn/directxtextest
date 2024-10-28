@echo off
call startup.cmd %0

echo.
echo -nologo -hflip -o %OUTTESTDIR% -px HF_ %REFDIR%\1234-bmp.bmp
%RUN% -nologo -hflip -o %OUTTESTDIR% -px HF_ %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -vflip -o %OUTTESTDIR% -px VF_ %REFDIR%\1234-bmp.bmp
%RUN% -nologo -vflip -o %OUTTESTDIR% -px VF_ %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -hflip -vflip -o %OUTTESTDIR% -px BF_ %REFDIR%\1234-bmp.bmp
%RUN% -nologo -hflip -vflip -o %OUTTESTDIR% -px BF_ %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -c FFFFFF -o %OUTTESTDIR% -px CK_ %REFDIR%\1234-bmp.bmp
%RUN% -nologo -c FFFFFF -o %OUTTESTDIR% -px CK_ %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --tonemap -o %OUTTESTDIR% -px TM_ %REFDIR%\hdrtest.dds
%RUN% -nologo --tonemap -o %OUTTESTDIR% -px TM_ %REFDIR%\hdrtest.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

for %%1 in (709to2020 709toHDR10 709toP3D65) do (echo.
echo -nologo --rotate-color %%1 -o %OUTTESTDIR% -sx _%%1 %REFDIR%\hdrtest.dds
%RUN% -nologo --rotate-color %%1 -o %OUTTESTDIR% -sx _%%1 %REFDIR%\hdrtest.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" ))

echo.
echo -nologo --swizzle bgra -o %OUTTESTDIR% -px BGRA_ %REFDIR%\1234-bmp.bmp
%RUN% -nologo --swizzle bgra -vflip -o %OUTTESTDIR% -px BGRA_ %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --swizzle rrra -o %OUTTESTDIR% -px RED_ %REFDIR%\1234-bmp.bmp
%RUN% -nologo --swizzle rrra -vflip -o %OUTTESTDIR% -px RED_ %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --swizzle ggga -o %OUTTESTDIR% -px GREEN_ %REFDIR%\1234-bmp.bmp
%RUN% -nologo --swizzle ggga -vflip -o %OUTTESTDIR% -px GREEN_ %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo --swizzle bbba -o %OUTTESTDIR% -px BLUE_ %REFDIR%\1234-bmp.bmp
%RUN% -nologo --swizzle bbba -vflip -o %OUTTESTDIR% -px BLUE_ %REFDIR%\1234-bmp.bmp
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )




