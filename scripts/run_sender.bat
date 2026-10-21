@echo off
setlocal
if not exist "%~dp0..\bin\sender.exe" (
    echo Sender is missing. Run scripts\build.bat first.
    exit /b 1
)
"%~dp0..\bin\sender.exe"
exit /b %errorlevel%
