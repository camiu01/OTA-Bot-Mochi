@echo off
setlocal
echo Exporting every Mochi face, one GIF per file...
call "%~dp0export-gifs.bat" --all %*
set "result=%errorlevel%"
if "%~1"=="" pause
exit /b %result%
