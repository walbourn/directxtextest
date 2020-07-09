@echo off
call startup.cmd %0

for /F "eol=#" %%r in (%REFDIR%\readfiletypes.txt) do call :readfiletype %%r
goto :EOF

:readfiletype
for /F "eol=#" %%w in (%REFDIR%\writefiletypes.txt) do call :writefiletype %1 %%w
echo.
echo -nologo -ft HDR -f R32G32B32_FLOAT -o %OUTTESTDIR% %REFDIR%\1234-%1.%1
"%RUN%" -nologo -ft HDR -f R32G32B32_FLOAT -o %OUTTESTDIR% %REFDIR%\1234-%1.%1
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
echo.
echo -nologo -ft PFM -f R32G32B32_FLOAT -o %OUTTESTDIR% %REFDIR%\1234-%1.%1
"%RUN%" -nologo -ft PFM -f R32G32B32_FLOAT -o %OUTTESTDIR% %REFDIR%\1234-%1.%1
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF

:writefiletype
echo.
echo -nologo -ft %2 -f R8G8B8A8_UNORM -o %OUTTESTDIR% %REFDIR%\1234-%1.%1
"%RUN%" -nologo -ft %2 -f R8G8B8A8_UNORM -o %OUTTESTDIR% %REFDIR%\1234-%1.%1
if %ERRORLEVEL% NEQ 0 ( call failure.cmd "Failed" )
goto :EOF
