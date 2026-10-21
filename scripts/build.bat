@echo off
setlocal
pushd "%~dp0.."
if errorlevel 1 exit /b 1

where cl >nul 2>&1
if errorlevel 1 goto :find_tools
goto :compile

:find_tools
set "SPL_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%SPL_VSWHERE%" goto :no_compiler
set "SPL_VS_INSTALL="
for /f "usebackq delims=" %%I in (`"%SPL_VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "SPL_VS_INSTALL=%%I"
if not defined SPL_VS_INSTALL goto :no_compiler
if not exist "%SPL_VS_INSTALL%\VC\Auxiliary\Build\vcvars64.bat" goto :no_compiler
call "%SPL_VS_INSTALL%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 goto :failed

:compile
if not exist build mkdir build
if not exist build goto :failed
if not exist bin mkdir bin
if not exist bin goto :failed
cl /nologo /std:c++17 /EHsc /W4 /Zi src\receiver.cpp /Fobuild\receiver.obj /Febin\receiver.exe /Fdbuild\receiver.pdb /link Ws2_32.lib
if errorlevel 1 goto :failed
cl /nologo /std:c++17 /EHsc /W4 /Zi src\sender.cpp /Fobuild\sender.obj /Febin\sender.exe /Fdbuild\sender.pdb /link Ws2_32.lib
if errorlevel 1 goto :failed
echo.
echo Build successful. Open two terminals and run:
echo   .\scripts\run_receiver.bat
echo   .\scripts\run_sender.bat
popd
exit /b 0

:no_compiler
echo MSVC was not found. Install Desktop development with C++ and a Windows SDK.
echo Or run this script from an x64 Native Tools Command Prompt.
goto :failed

:failed
echo Build failed. Read the first compiler or linker error above.
popd
exit /b 1
