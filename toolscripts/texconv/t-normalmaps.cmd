@echo off
call startup.cmd %0

for /F "eol=#" %%w in (%REFDIR%\formatsnm.txt) do call :writeformat %%w
goto :EOF

:writeformat
echo.
echo -nologo -nmap lo -nmapamp 4 -f %1 -ft dds -sx %1 -o %OUTTESTDIR% %REFDIR%\heightmap.dds
%RUN% -nologo -nmap lo -nmapamp 4 -f %1 -ft dds -sx %1 -o %OUTTESTDIR% %REFDIR%\heightmap.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF

