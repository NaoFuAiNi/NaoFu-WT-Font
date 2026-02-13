# 资源文件目录 (assets)

项目用到的字体、图片之类的都放这儿。

## 目录说明

- **字体**：可放置开源/授权字体文件（如 `.ttf`、`.otf`），供 UI 或文档使用。
- **图片**：可放置图标、插图等（如 `.png`、`.svg`）。

## 当前资源

- `CuteFount.ttf` — 站酷快乐体开源版（ZCOOL Kuaile），**项目全局 UI 字体**，标题栏文字也会优先使用。  
  编译 Launcher 时会随 `assets` 一起复制到输出目录，分发给别人时请保留输出目录下的 `assets` 文件夹，否则字体会回退到系统默认。
- `app_ico.png` / `app-icon.png` — 标题栏左上角小 logo。优先使用 `app_ico.png`（下划线），若不存在则用 `app-icon.png`（连字符）。放其中任意一个即可。
- `author_avatar.jpg` — 「关于作者」页与感谢名单中使用的作者头像。请将你的头像复制到本目录并命名为 `author_avatar.jpg`；若不存在，关于页会隐藏头像区域。