@echo off
setlocal
if not exist "%~dp0..\bin\receiver.exe" (
    echo Receiver is missing. Run scripts\build.bat first.
    exit /b 1
)
"%~dp0..\bin\receiver.exe"
exit /b %errorlevel%
