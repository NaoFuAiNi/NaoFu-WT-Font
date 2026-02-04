@echo off
REM 生成 nf_subset_tool.exe（单文件，无控制台窗口，主程序调用时不需要用户装 Python）
pip install -r requirements.txt
pyinstaller --onefile --noconsole --name nf_subset_tool subset_tool.py
echo.
echo 生成文件: dist\nf_subset_tool.exe
echo 将其复制到主程序同目录，或放入 src/resource/ 供主程序内嵌。
pause
