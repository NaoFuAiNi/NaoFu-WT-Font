@echo off
chcp 65001 >nul
REM 自动查找 VS 并编译根目录用启动器 NaoFu WT Hub.exe。在 debug 目录执行: scripts\build_launcher_stub.bat
cd /d "%~dp0.."

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSINSTALL="
if exist "%VSWHERE%" for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do set "VSINSTALL=%%i"
if not defined VSINSTALL set "VSINSTALL=C:\Program Files\Microsoft Visual Studio\2022\Community"
if not exist "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" (
    echo [ERROR] Visual Studio not found. Please install VS with "Desktop development with C++" workload.
    exit /b 1
)
call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

if not exist "src\obj" mkdir "src\obj"
cl /nologo /W3 /O2 /DUNICODE /D_UNICODE /Fo"src\obj\\" /Fe"NaoFu WT Hub.exe" src\source\launcher_stub.c /link /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup
if errorlevel 1 (echo BUILD FAILED & exit /b 1)
echo BUILD OK: NaoFu WT Hub.exe
