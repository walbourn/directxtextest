@echo off
call startup.cmd %0

for /F "eol=#" %%w in (%REFDIR%\bc11.txt) do call :writeformat %%w
for /F "eol=#" %%w in (%REFDIR%\bcuniform.txt) do call :writeuniform %%w
for /F "eol=#" %%w in (%REFDIR%\bcdither.txt) do call :writedither %%w
for /F "eol=#" %%w in (%REFDIR%\bcmax.txt) do call :writemax %%w
for /F "eol=#" %%w in (%REFDIR%\bcgpu.txt) do call :writegpu %%w
goto :EOF

:writeformat
echo.
echo -nologo -f %1 -ft dds -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
"%RUN%" -nologo -f %1 -ft dds -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF

:writeuniform
echo.
echo -nologo -f %1 -ft dds -bcuniform -px bcu -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
"%RUN%" -nologo -f %1 -ft dds -bcuniform -px bcu -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF

:writedither
echo.
echo -nologo -f %1 -ft dds -bcdither -px bcd -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
"%RUN%" -nologo -f %1 -ft dds -bcdither -px bcd -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF

:writemax
echo.
echo -nologo -f %1 -ft dds -bcmax -px bcm -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
"%RUN%" -nologo -f %1 -ft dds -bcmax -px bcm -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF

:writegpu
echo.
echo -nologo -f %1 -ft dds -nogpu -px nogpu -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
"%RUN%" -nologo -f %1 -ft dds -nogpu -px nogpu -sx %1 -o %OUTTESTDIR% %REFDIR%\1234-dds.dds
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF
