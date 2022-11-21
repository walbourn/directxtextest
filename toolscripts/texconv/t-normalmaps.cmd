@echo off
call startup.cmd %0

for /F "eol=#" %%w in (%REFDIR%\formatsnm.txt) do call :writeformat %%w

echo.
echo -nologo -nmap lo -nmapamp 4 -f BC5_SNORM -ft dds -sx BC5_SNORM -o %OUTTESTDIR% %REFDIR%\heightmap.dds
%RUN% -nologo -nmap lo -nmapamp 4 -f BC5_SNORM -ft dds -sx BC5_SNORM -o %OUTTESTDIR% %REFDIR%\heightmap.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -x2bias -nmap lo -nmapamp 4 -f BC5_UNORM -ft dds -sx BC5_UNORM -o %OUTTESTDIR% %REFDIR%\heightmap.dds
%RUN% -nologo -x2bias -nmap lo -nmapamp 4 -f BC5_UNORM -ft dds -sx BC5_UNORM -o %OUTTESTDIR% %REFDIR%\heightmap.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -reconstructz -f R8G8B8A8_UNORM -ft dds -px RZ -o %OUTTESTDIR% %OUTTESTDIR%\heightmapBC5_UNORM.dds
%RUN% -nologo -reconstructz -f R8G8B8A8_UNORM -ft dds -px RZ -o %OUTTESTDIR% %OUTTESTDIR%\heightmapBC5_UNORM.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

echo.
echo -nologo -x2bias -inverty -nmap lo -nmapamp 4 -f BC5_UNORM -ft dds -px IY -sx BC5_UNORM -o %OUTTESTDIR% %REFDIR%\heightmap.dds
%RUN% -nologo -x2bias -inverty -nmap lo -nmapamp 4 -f BC5_UNORM -ft dds -px IY -sx BC5_UNORM -o %OUTTESTDIR% %REFDIR%\heightmap.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )

goto :EOF

:writeformat
echo.
echo -nologo -nmap lo -nmapamp 4 -f %1 -ft dds -sx %1 -o %OUTTESTDIR% %REFDIR%\heightmap.dds
%RUN% -nologo -nmap lo -nmapamp 4 -f %1 -ft dds -sx %1 -o %OUTTESTDIR% %REFDIR%\heightmap.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF

