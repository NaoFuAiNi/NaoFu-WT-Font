# tools/nf_subset_tool

字体**子集化 + 西里尔→拉丁映射**的独立小工具，供 C 主程序调用。

## 组成

- `subset_tool.py`：核心逻辑（使用 `fontTools` 提供的 `TTFont` 与 `subset` 模块）。  
  - 支持两种用法：  
    - `subset_tool.py <input_font> <output_font>`：仅做西里尔 `Т/У/Е` → 拉丁 `T/U/E` 映射；  
    - `subset_tool.py <ref_font> <input_font> <output_font>`：按参考字体字符集做子集化，并附带映射。  
- `build_exe.bat`：使用 PyInstaller 打包为 `nf_subset_tool.exe`。

## 打包为 exe

在本目录执行（需要 Python 3 + pip）：  

```bat
build_exe.bat
```

或等价命令：

```bat
pip install -r requirements.txt
pyinstaller --onefile --noconsole --name nf_subset_tool subset_tool.py
```

生成的 `dist/nf_subset_tool.exe` 拷到：

- 开发：`debug/` 根目录，这样跑 `build_debug.ps1` 或编 Launcher 时会自动带过去；  
- 发布：和 `NaoFu WT Customize Font 2.10.exe` 放一起，或丢发布包的 `tools/` 里。

