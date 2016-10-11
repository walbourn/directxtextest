@echo off
call startup.cmd %0

for /F "eol=#" %%r in (%REFDIR%\filters.txt) do call :dofilter %%r
goto :EOF

:dofilter
echo.
echo -nologo -if %1 -ft DDS -sx _%1 -o %OUTTESTDIR% %REFDIR%\1234-BMP.BMP
"%RUN%" -nologo -if %1 -ft DDS -sx _%1 -o %OUTTESTDIR% %REFDIR%\1234-BMP.BMP
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF

