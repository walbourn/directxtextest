@echo off
call startup.cmd %0

for /F "eol=#" %%w in (%REFDIR%\formats.txt) do call :writeformat %%w
goto :EOF

:writeformat
echo.
echo -nologo -f %1 -ft dds -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
"%RUN%" -nologo -f %1 -ft dds -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

if %1.==R10G10B10A2_UNORM. (echo.
echo -nologo -f %1 -dx9 -ft dds -px DX9_ -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
"%RUN%" -nologo -f %1 -dx9 -ft dds -px DX9_ -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" ))

if %1.==B5G5R5A1_UNORM. (echo.
echo -nologo -f %1 -dx10 -ft dds -px DX10_ -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
"%RUN%" -nologo -f %1 -dx10 -ft dds -px DX10_ -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" ))

goto :EOF

