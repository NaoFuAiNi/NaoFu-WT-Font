/* nf_subset.h - 调用 nf_subset_tool 瘦身/remap */

#ifndef NF_SUBSET_H
#define NF_SUBSET_H

#include "nf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 运行瘦身/remap 工具。ref_path 非空时：瘦身+西里尔 remap；ref_path 为 NULL 时：仅西里尔→拉丁 remap。成功返回 0，失败返回 -1。 */
s32 NF_runSubsetTool(const char* ref_path, const char* input_path, const char* output_path);

#ifdef __cplusplus
}
#endif

#endif /* NF_SUBSET_H */
