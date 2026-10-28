@echo off
setlocal
where py >nul 2>nul
if errorlevel 1 (
	echo Python launcher not found. Install Python 3 with the py launcher.
	pause
	exit /b 1
)
if "%~1"=="" (
	py -3 "%~dp0export-gifs.py" --interactive
) else (
	py -3 "%~dp0export-gifs.py" %*
)
set "result=%errorlevel%"
if "%~1"=="" pause
exit /b %result%
