@echo off
setlocal
echo Exporting every face in all four styles, one folder per style...
call "%~dp0export-gifs.bat" --all --all-styles %*
set "result=%errorlevel%"
if "%~1"=="" pause
exit /b %result%
