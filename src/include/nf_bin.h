/* nf_bin.h - vromfs 内二进制块查找 */

#ifndef NF_BIN_H
#define NF_BIN_H

#include "nf_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 在 haystack 中查找 needle 首次出现位置，返回指针；未找到返回 NULL。 */
const NF_byte_t* NF_findSubsequence(const NF_byte_t* haystack, size_t haystack_len,
                                    const NF_byte_t* needle, size_t needle_len);

#ifdef __cplusplus
}
#endif

#endif /* NF_BIN_H */
