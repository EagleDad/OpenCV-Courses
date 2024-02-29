ECHO OFF

REM CreateProjectMsvc.bat -cov -bench -dir buildMsvc -woapp OFF -woclr ON

SETLOCAL EnableDelayedExpansion
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (set "DEL=%%a")

REM Default config
SET VS_VERSION="Visual Studio 17 2022"
SET CMAKE_FLAGS=
SET BUILD_DIR=buildMsvc

CALL InitializeVariables.bat

CD %PROJECT_ROOT%

:LOOP
IF NOT "%1"=="" (
    IF "%1"=="-cov" (
		ECHO Code coverage is activ
		SET CMAKE_FLAGS=%CMAKE_FLAGS% -D COVERAGE_ON=ON
    )
    IF "%1"=="-bench" (
		ECHO Benchmark is active
		SET CMAKE_FLAGS=%CMAKE_FLAGS% -D BUILD_WITH_BENCHMARK=ON
    )
    IF "%1"=="-dir" (
        SET BUILD_DIR=%2
        ECHO Build directory is set to %2%
		SHIFT
    )
	IF "%1"=="-woapp" (
	    ECHO Without applications is %2
		SET CMAKE_FLAGS=%CMAKE_FLAGS% -D BUILD_WITHOUT_APPLICATIONS=%2
		SHIFT
    )
	IF "%1"=="-woclr" (
	    ECHO Without clr is %2
		SET CMAKE_FLAGS=%CMAKE_FLAGS% -D BUILD_WITHOUT_CLR=%2
		SHIFT
    )
    SHIFT
    GOTO :LOOP
)

:MAIN

ECHO %CMAKE_FLAGS%

REM copy %PROJECT_ROOT%\.conan\debug_ipt %USERPROFILE%\.conan\debug_ocv

REM copy %PROJECT_ROOT%\.conan\release_ipt %USERPROFILE%\.conan\release_ocv

CALL conan install %PROJECT_ROOT%\.conan --output-folder=%PROJECT_ROOT%\%BUILD_DIR% --build=missing --profile=%PROJECT_ROOT%\.conan\debug_ocv

CALL conan install %PROJECT_ROOT%\.conan --output-folder=%PROJECT_ROOT%\%BUILD_DIR% --build=missing --profile=%PROJECT_ROOT%\.conan\release_ocv

CALL %PROJECT_ROOT%\%BUILD_DIR%\conanbuild.bat

CALL %PROJECT_ROOT%\%BUILD_DIR%\conanrun.bat

REM CALL cmake -S . -B %PROJECT_ROOT%\%BUILD_DIR% -G %VS_VERSION% %CMAKE_FLAGS% -D CMAKE_TOOLCHAIN_FILE=%PROJECT_ROOT%\%BUILD_DIR%\conan_toolchain.cmake 
CALL cmake -S . -B %PROJECT_ROOT%\%BUILD_DIR% -G %VS_VERSION% -D BUILD_TESTING=OM -D CMAKE_TOOLCHAIN_FILE=%PROJECT_ROOT%\%BUILD_DIR%\conan_toolchain.cmake 


REM CALL cmake -S . -B %PROJECT_ROOT%\%BUILD_DIR% --preset conan-default

IF ERRORLEVEL 1 ( GOTO :FAILED )

GOTO :SUCCESS

:SUCCESS
CALL :colorEcho 2F "Creating %BUILD_DIR% finished successfully!"
GOTO :END

:FAILED
CALL :colorEcho 4E "Creating %BUILD_DIR% failed!"
GOTO :END

:colorEcho
echo off
<nul set /p ".=%DEL%" > "%~2"
findstr /v /a:%1 /R "^$" "%~2" nul
del "%~2" > nul 2>&1
goto :eof

:END
CD %PROJECT_ROOT%/scripts