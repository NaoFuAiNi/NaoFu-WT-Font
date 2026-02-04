/* nf_patcher.h - 字体替换流程 */

#ifndef NF_PATCHER_H
#define NF_PATCHER_H

#include "nf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 执行字体替换流程。font_choice 为 1~g_num_font_choices 的序号；project_dir 为本项目输出目录；auto_slim 非 0 时若字体大于原版则直接瘦身不询问。成功返回 0，失败返回 -1。 */
s32 NF_runFontReplace(u32 font_choice, const char* project_dir, int auto_slim);

#ifdef __cplusplus
}
#endif

#endif /* NF_PATCHER_H */
