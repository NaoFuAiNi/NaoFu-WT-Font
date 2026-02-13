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

# 被 C 调起来时 stdout/stderr 可能没挂好，兜个底
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

# 苏系载具名会用西里尔 Т/У/Е，没这仨就缺字，用拉丁 T/U/E 顶
CYRILLIC_TO_LATIN = [
    (0x0422, 0x0054),  # Т → T
    (0x0423, 0x0055),  # У → U
    (0x0415, 0x0045),  # Е → E
]

def remap_cyrillic_to_latin(font):
    """往 cmap 里加 Т/У/Е → T/U/E。只动 Unicode 子表，format 0 那种别动，不然保存会炸。"""
    cmap = font.getBestCmap()
    if not cmap:
        return

    glyph_for = {}
    for _cyr, lat in CYRILLIC_TO_LATIN:
        if lat in cmap:
            glyph_for[_cyr] = cmap[lat]
    if not glyph_for:
        return

    cmap_table = font.get("cmap")
    if not cmap_table:
        return

    for table in getattr(cmap_table, "tables", []):
        if not hasattr(table, "cmap") or not isinstance(table.cmap, dict):
            continue

        # 只改 Unicode 子表
        is_unicode = (
            table.platformID == 0
            or (table.platformID == 3 and getattr(table, "platEncID", None) in (1, 10))
        )
        if not is_unicode:
            continue

        # format 0 只支持 0~255，塞 0x0422 会炸
        fmt = getattr(table, "format", None)
        if fmt == 0:
            continue

        for cyr, gname in glyph_for.items():
            table.cmap[cyr] = gname

def main():
    argc = len(sys.argv)
    if argc == 3:
        # 只做 remap
        input_path = sys.argv[1]
        output_path = sys.argv[2]
        ref_path = None
    elif argc == 4:
        # 瘦身 + remap
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
            # 有的字体子集化后重算 UnicodeRanges 会越界报错，关掉不碍事
            options.prune_unicode_ranges = False
            subsetter = subset_module.Subsetter(options)
            subsetter.populate(unicodes=unicodes)
            subsetter.subset(font)
        remap_cyrillic_to_latin(font)
        font.save(output_path)
        font.close()
    except Exception as e:
        import traceback
        sys.stderr.write("Error: %s\n" % str(e))
        traceback.print_exc(file=sys.stderr)
        sys.exit(1)
    sys.exit(0)

if __name__ == "__main__":
    main()
