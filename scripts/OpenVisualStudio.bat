ECHO OFF

REM OpenVisualStudio.bat -dir buildMSVC

SETLOCAL EnableDelayedExpansion
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (set "DEL=%%a")

SET BUILD_DIR=buildMsvc

CALL InitializeVariables.bat

CD %PROJECT_ROOT%

:LOOP
IF NOT "%1"=="" (
    IF "%1"=="-dir" (
        SET BUILD_DIR=%2
        ECHO Build directory is set to %2%
		SHIFT
    )
    SHIFT
    GOTO :LOOP
)

:MAIN

CALL "%VS2022INSTALLDIR%\Common7\IDE\devenv.exe" "%PROJECT_ROOT%\%BUILD_DIR%\ImageProcessingToolbox.sln"