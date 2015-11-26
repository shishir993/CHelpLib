@ECHO OFF

REM Usage : 
REM                   %1             %2                %3               %4
REM   publish.cmd source_headers source_binaries platform_string publish_rootPath

IF "%1" == "" goto missing_args
IF "%2" == "" goto missing_args
IF "%3" == "" goto missing_args
IF "%4" == "" goto missing_args

SET _srcInc=%1
SET _srcBin=%2
SET _plat=%3
if "%_plat%" == "Win32" (
  SET _plat=x86
)

SET _dest=%4\published
SET _destBin=%_dest%\bin_%_plat%
SET _destInc=%_dest%\include

ECHO Copying from %_srcInc% to %_dest%

REM If destination root directory doesn't exist, create it
IF NOT EXIST %_dest% (
  
  MKDIR %_dest%
  
) ELSE (
  
  REM Remove existing bin and inc directories.
  REM Do not remove the root destination directory because both x86 and x64 binaries 
  REM   are present in the same root directory. VS builds only one platform at a time.
  IF EXIST %_destBin% (
    RMDIR /q /s %_destBin%
  )

  IF EXIST %_destInc% (
    RMDIR /q /s %_destInc%
  )

)

REM Create destination bin and include directories
MKDIR %_destBin%
MKDIR %_destInc%

REM Copy the headers
COPY /Y %_srcInc%\*.h %_destInc%
ECHO Copied headers. Error code = %ERRORLEVEL%

REM Copy the binaries
COPY /Y %2\CHelpLib.dll %_destBin%
COPY /Y %2\CHelpLib.lib %_destBin%
ECHO Copied binaries. Error code = %ERRORLEVEL%
goto eof

:missing_args
ECHO Missing arguments
ECHO Arg1 = %1
ECHO Arg2 = %2
ECHO Arg3 = %3
ECHO Arg4 = %4
ECHO Usage:
ECHO   publish.cmd source_headers source_binaries platform_string publish_rootPath
goto eof

:eof
@ECHO ON
