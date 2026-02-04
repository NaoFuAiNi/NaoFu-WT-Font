/* nf_types.h - 公共类型 */

#ifndef NF_TYPES_H
#define NF_TYPES_H

#include <stdint.h>
#include <stdio.h>

/* 固定宽度整型（便于跨平台与代码意图表达） */
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;

typedef u8        NF_byte_t;
typedef long      NF_file_size_t;
typedef FILE*     NF_file_handle_t;

typedef struct {
    u32            id;
    const char*    display_name;
    const char*    original_filename;
} NF_FontChoice_t;

#endif /* NF_TYPES_H */
