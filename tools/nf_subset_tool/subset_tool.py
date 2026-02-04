#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
字体瘦身 + 西里尔→拉丁 remap 工具。
用法1（瘦身+remap）: subset_tool.py <参考字体.ttf> <待瘦身字体.ttf> <输出字体.ttf>
用法2（仅 remap）: subset_tool.py <输入字体.ttf> <输出字体.ttf>
remap: 西里尔 Т(U+0422)/У(U+0423)/Е(U+0415) 使用拉丁 T/U/E 字形，解决苏系载具名称缺字。
成功退出码 0，失败 1。
"""
import sys
import os

# 从子进程启动时（如 C 程序 CreateProcess）可能为 None，尽量用可被父进程捕获的句柄
def _safe_stream(name, default_fd):
    s = sys.__dict__.get(name)
    if s is not None and hasattr(s, "write"):
        return s
    try:
        return os.fdopen(default_fd, "w")
    except Exception:
        return open(os.devnull, "w")
if sys.stdout is None:
    sys.stdout = _safe_stream("stdout", 1)
if sys.stderr is None:
    sys.stderr = _safe_stream("stderr", 2)

# 西里尔→拉丁：游戏里苏系名称可能用西里尔 Т/У/Е，字体无这些字会缺字，映射到拉丁 T/U/E 字形
CYRILLIC_TO_LATIN = [
    (0x0422, 0x0054),  # Т → T
    (0x0423, 0x0055),  # У → U
    (0x0415, 0x0045),   # Е → E
]

def remap_cyrillic_to_latin(font):
    """在字体 cmap 中增加西里尔 Т/У/Е → 拉丁 T/U/E 字形映射。"""
    cmap = font.getBestCmap()
    if not cmap:
        return
    glyph_for = {}
    for _cyr, lat in CYRILLIC_TO_LATIN:
        if lat in cmap:
            glyph_for[_cyr] = cmap[lat]
    if not glyph_for:
        return
    for table in getattr(font.get("cmap"), "tables", []):
        if hasattr(table, "cmap") and isinstance(table.cmap, dict):
            for cyr, gname in glyph_for.items():
                table.cmap[cyr] = gname

def main():
    argc = len(sys.argv)
    if argc == 3:
        # 仅 remap：input → output
        input_path = sys.argv[1]
        output_path = sys.argv[2]
        ref_path = None
    elif argc == 4:
        # 瘦身 + remap：ref, input → output
        ref_path = sys.argv[1]
        input_path = sys.argv[2]
        output_path = sys.argv[3]
    else:
        sys.stderr.write("Usage: subset_tool.py <input_font> <output_font>  (remap only)\n")
        sys.stderr.write("   or: subset_tool.py <ref_font> <input_font> <output_font>  (subset + remap)\n")
        sys.exit(1)

    if not os.path.isfile(input_path):
        sys.stderr.write("Error: input font not found: %s\n" % input_path)
        sys.exit(1)
    if ref_path is not None and not os.path.isfile(ref_path):
        sys.stderr.write("Error: ref font not found: %s\n" % ref_path)
        sys.exit(1)

    try:
        from fontTools.ttLib import TTFont
        from fontTools import subset as subset_module
    except ImportError:
        sys.stderr.write("Error: fonttools not found (bundled exe should include it)\n")
        sys.exit(1)

    try:
        font = TTFont(input_path)
        if ref_path is not None:
            ref = TTFont(ref_path)
            rcmap = ref.getBestCmap()
            ref.close()
            if not rcmap:
                sys.stderr.write("Error: ref font has no cmap\n")
                font.close()
                sys.exit(1)
            unicodes = set(rcmap.keys())
            options = subset_module.Options()
            subsetter = subset_module.Subsetter(options)
            subsetter.populate(unicodes=unicodes)
            subsetter.subset(font)
        remap_cyrillic_to_latin(font)
        font.save(output_path)
        font.close()
    except Exception as e:
        sys.stderr.write("Error: %s\n" % str(e))
        sys.exit(1)
    sys.exit(0)

if __name__ == "__main__":
    main()
