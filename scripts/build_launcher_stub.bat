@echo off
chcp 65001 >nul
REM 在「x64 本机工具命令提示」下运行，生成根目录用启动器 NaoFu WT Hub.exe。在 debug 目录执行: scripts\build_launcher_stub.bat
cd /d "%~dp0.."
if not exist "src\obj" mkdir "src\obj"
cl /nologo /W3 /O2 /DUNICODE /D_UNICODE /Fo"src\obj\\" /Fe"NaoFu WT Hub.exe" src\source\launcher_stub.c /link /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup
if errorlevel 1 (echo 编译失败 & exit /b 1)
echo 构建成功: NaoFu WT Hub.exe
