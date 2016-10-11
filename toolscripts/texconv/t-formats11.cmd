@echo off
call startup.cmd %0

for /F "eol=#" %%w in (%REFDIR%\formats11.txt) do call :writeformat %%w
goto :EOF

:writeformat
echo.
echo -nologo -f %1 -ft dds -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
"%RUN%" -nologo -f %1 -ft dds -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF

