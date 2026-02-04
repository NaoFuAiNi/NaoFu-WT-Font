@echo off
REM 自动设置 VS 环境并编译 C 主程序。在 debug 目录执行: scripts\do_build.cmd
REM 若 VS 安装路径不同，请修改下面一行
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d "%~dp0.."
cl /nologo /W3 /O2 /utf-8 /I src\include /Fe"NaoFu WT Customize Font 2.10.exe" src\source\main.c src\source\nf_bin.c src\source\nf_console.c src\source\nf_fonts.c src\source\nf_io.c src\source\nf_patcher.c src\source\nf_subset.c src\source\nf_ui.c
if errorlevel 1 (echo BUILD FAILED & exit /b 1)
echo BUILD OK: NaoFu WT Customize Font 2.10.exe
