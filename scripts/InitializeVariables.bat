ECHO OFF

SET NINJA_PATH=%cd%/../.ninja
SET PROJECT_ROOT=%cd%/../

IF "%INIT%"=="" GOTO :NOT_INITIALIZED

GOTO :END

:NOT_INITIALIZED
REM Add nuget to the local path to be able to build the packages
ECHO Init variables

REM SET PATH=%PATH%;%NUGET_PATH%;%NINJA_PATH%;
SET PATH=%PATH%;%NINJA_PATH%;

SET INIT=1

REM CALL InitIntelEnvironment.bat

REM CALL InitMsvcEnvironment.bat

:END