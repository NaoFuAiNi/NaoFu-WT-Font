# 如何编译成 Release 版本

本文说明如何从本仓库编译出可发给用户的 **Release（发布）包**。

若从 GitHub 克隆本仓库，则仓库根目录即文档中的「debug」目录，所有脚本均在仓库根目录执行即可。

## 一、前置条件

- **.NET 8 SDK**：用于编译 Launcher（C#）
- **Visual Studio（含 C++ 本机工具）**：用于编译 C 主程序与根目录启动器（NaoFu WT Hub.exe）
- **Python 3**（可选）：仅当需要重新打包 **nf_subset_tool.exe** 时使用

## 二、一键生成 Release 目录

在**项目根目录**下执行（PowerShell）：

```powershell
.\debug\scripts\build_release.ps1
```

或在 **debug** 目录下执行：

```powershell
.\scripts\build_release.ps1
```

脚本会：

1. 清空并创建 **release** 目录
2. 发布 Launcher（Release、win-x64、自包含）到 **release/bin**
3. 从 **debug** 复制 **NaoFu WT Customize Font 2.10.exe**、**nf_subset_tool.exe** 到 **release/tools**
4. 复制 **ui**、**assets**、**font** 等（排除内部 .md）
5. 复制根目录启动器 **NaoFu WT Hub.exe** 到 **release** 根目录
6. 将 **debug/README.md** 复制为 **release/README.md**（给最终用户看的说明）

完成后，将 **release** 文件夹打包即可分发。用户解压后双击 **release/NaoFu WT Hub.exe** 即可运行。

## 三、发布前需要先编译好的文件

脚本会从 **debug** 目录复制以下文件，因此请先确保它们已存在且为最新：

| 文件 | 如何生成 |
|------|----------|
| **NaoFu WT Customize Font 2.10.exe** | 在「x64 本机工具命令提示」下进入 **debug**，执行 `scripts\build.bat` 或 `scripts\build_c.bat` |
| **NaoFu WT Hub.exe** | 在「x64 本机工具命令提示」下进入 **debug**，执行 `scripts\build_launcher_stub.bat` |
| **Launcher** | 在项目根或 debug 下执行 `dotnet build debug\Launcher\NaoFu.WT.Font.Launcher.csproj -c Release`（脚本内部会执行 dotnet publish，无需手动 publish） |
| **nf_subset_tool.exe** | 进入 **debug/tools/nf_subset_tool**，执行 `build_exe.bat`，再将 **dist/nf_subset_tool.exe** 复制到 **debug** 根目录 |

## 四、可选：发布前清理

若希望 release 包更干净，可在执行 **build_release.ps1** 前先清理 debug 下的编译产物与用户数据（详见下方）。**非必须**，脚本会按需创建目录并复制现有内容。

- **Launcher**：可先删 **debug/Launcher/obj**、**debug/Launcher/bin**，再重新 `dotnet build/publish`（由脚本完成）
- **build/** 下子目录、**font/builtin/** 下内容、**font/custom/** 下文件、**nf_config.json**：为用户使用后产生，删除后 release 中对应目录为空，用户首次使用无影响
- **.obj 文件**：C 编译中间文件，可删，下次执行 build.bat 会重新生成

## 五、release 与 Release 目录说明

- 脚本生成的目录名为 **release**（小写），位于项目根目录下
- 若你已有 **Release**（大写）目录，两者不同；脚本不会修改 **Release**，只维护 **release**
- 打包分发时，将 **release** 整个压缩为 zip 或重命名为 **Release** 均可

---

**简要步骤汇总**：先按「三」编译好 C 程序、启动桩和（可选）nf_subset_tool，再在项目根执行 `.\debug\scripts\build_release.ps1`，最后打包 **release** 目录即可。
