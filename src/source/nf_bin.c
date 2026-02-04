/* nf_bin.c - 二进制块查找 */

#include "nf_bin.h"
#include <string.h>

const NF_byte_t* NF_findSubsequence(const NF_byte_t* haystack, size_t haystack_len,
                                    const NF_byte_t* needle, size_t needle_len) {
    if (needle_len == 0) return haystack;
    if (haystack_len < needle_len) return NULL;
    for (size_t i = 0; i <= haystack_len - needle_len; ++i) {
        if (memcmp(haystack + i, needle, needle_len) == 0) {
            return haystack + i;
        }
    }
    return NULL;
}
