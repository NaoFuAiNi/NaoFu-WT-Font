/* nf_fonts.c - 字体选项表 */

#include "nf_fonts.h"

const NF_FontChoice_t g_font_choices[] = {
    {1,  "默认常规字体          (通用文本)", "default_normal.otf"},
    {2,  "微软雅黑字体子集      (游戏中文显示)", "msyh.subset.ttf"},
    {3,  "Fira Sans Regular     (现代无衬线)", "firasanscondensed-regular.ttf"},
    {4,  "Fira Sans Light       (现代无衬线-轻量)", "firasanscondensed-light.ttf"},
    {5,  "JetBrains Mono        (程序员等宽字体)", "jetbrains-mono-nl-regular.ttf"},
    {6,  "M+ 2p                 (日文字体子集)", "mplus-2p-regular.subset.ttf"},
    {7,  "Nanum Gothic          (韩文字体子集)", "nanumgothicregular.subset.ttf"},
    {8,  "Open Sans Emoji       (带表情符号)", "opensansemoji.ttf"},
    {9,  "PT Sans Light         (轻量无衬线)", "pt_sans-light.otf"},
    {10, "PT Sans 网络版        (高度优化)", "pt_sans-web-regular-reducedheight.ttf"},
    {11, "稀有符号字体", "rare_symbols.ttf"},
    {12, "Skyquake游戏符号字体", "symbols_skyquake.ttf"},
    {13, "飞机相关HUD界面字体", "characters_aircrafts_hud.ttf"},
    {14, "数字风格字体", "characters_digital.ttf"},
    {15, "Apache直升机HUD专用字体", "characters_hud_apache.ttf"},
    {16, "F-14战斗机HUD专用字体", "characters_hud_f14.ttf"},
    {17, "JAS 39\"鹰狮\"战斗机HUD字体", "characters_hud_jas39.ttf"},
    {18, "MiG-29战斗机HUD字体", "characters_hud_mig_29.ttf"},
    {19, "Mirage 2000战斗机HUD字体", "characters_hud_mirage_2000.ttf"},
    {20, "苏联机型HUD字体", "characters_hud_ussr.ttf"},
    {21, "\"黑客帝国\"风格矩阵字体", "characters_matrix.ttf"}
};
const u32 g_num_font_choices = (u32)(sizeof(g_font_choices) / sizeof(g_font_choices[0]));
