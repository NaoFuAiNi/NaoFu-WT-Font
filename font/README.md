# font/

存放与字体相关的所有资源，分为三类：

- `source/`：游戏原始字体 bin 与解包后的字体文件（制作时的「参照源」）。  
- `builtin/`：内置与导入字体，每个字体一个子目录，内含 `fonts.vromfs.bin` 与 `DATA.NaoFu`。  
- `custom/`：当前制作用字体，界面中选择/拖入的字体会复制为 `MyFonts.ttf`。

发布或打包前：

- `source/` 和 `builtin/` 一般需要保留；  
- `custom/` 中的内容可以清空，只保留空目录，留给用户自己放字体。

详细说明看根目录 `RESOURCES.md`。

