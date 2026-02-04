# 构建与维护脚本说明

本目录集中存放 debug 相关的 `.ps1`、`.bat`、`.cmd` 脚本，便于管理和查找。

| 脚本 | 用途 | 运行方式 |
|------|------|----------|
| **build_release.ps1** | 生成发布版：清理并生成项目根下的 `release` 目录（Launcher 发布、复制 tools/ui/assets、根目录启动器等） | 在**项目根目录**执行：`.\debug\scripts\build_release.ps1`；或在 **debug** 下：`.\scripts\build_release.ps1` |
| **backup_project.ps1** | 将整个项目打包为 zip 备份到项目上级目录，文件名含日期 | 在**项目根目录**执行：`.\debug\scripts\backup_project.ps1`；或在 **debug** 下：`.\scripts\backup_project.ps1` |
| **build.bat** | 编译 C 主程序（NaoFu WT Customize Font 2.10.exe）。需在 **x64 本机工具命令提示** 下运行 | 在 **debug** 目录执行：`scripts\build.bat` |
| **build_c.bat** | 同上，但先自动调用 VS 的 vcvars64，无需单独打开开发者命令提示。路径中的 VS 安装目录请按本机修改 | 在 **debug** 目录执行：`scripts\build_c.bat` |
| **build_launcher_stub.bat** | 编译根目录用启动器 NaoFu WT Hub.exe（launcher_stub.c）。需在 x64 本机工具命令提示下运行 | 在 **debug** 目录执行：`scripts\build_launcher_stub.bat` |
| **do_build.cmd** | 与 build_c.bat 类似，自动设置 VS 环境并编译 C 主程序。VS 路径请按本机修改 | 在 **debug** 目录执行：`scripts\do_build.cmd` |

**说明：**

- `build_release.ps1`、`backup_project.ps1` 会根据脚本所在位置自动解析项目根目录（debug 的父目录），在项目根或 debug 下执行均可。
- C 相关脚本会先 `cd` 到上一级（即 debug 目录）再执行编译，因此请在 **debug** 目录下运行（例如 `scripts\build.bat`），不要进入 `scripts` 后双击运行。
- **nf_subset_tool** 的打包脚本仍保留在 `debug\tools\nf_subset_tool\build_exe.bat`，因需在该目录下执行 pip/pyinstaller。
