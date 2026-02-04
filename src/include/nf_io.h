/* nf_io.h - 文件读写 */

#ifndef NF_IO_H
#define NF_IO_H

#include "nf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 将整个文件读入内存，调用方负责 free(*buffer_ptr)。路径为 UTF-8（Windows 下支持中文）。失败返回 -1，成功返回 0。 */
s32 NF_readFileToBuffer(const char* filename, NF_byte_t** buffer_ptr, NF_file_size_t* file_size_ptr);

/* 将缓冲区写入文件。路径为 UTF-8（Windows 下支持中文）。失败返回 -1，成功返回 0。 */
s32 NF_writeBufferToFile(const char* filename, const NF_byte_t* buffer, NF_file_size_t size);

/* 路径是否存在（文件或目录）。路径为 UTF-8。返回 1 存在，0 不存在。 */
int NF_pathExists(const char* path_utf8);

/* 创建单层目录。路径为 UTF-8。成功或已存在返回 0，失败返回 -1。 */
s32 NF_createDir(const char* path_utf8);

#ifdef __cplusplus
}
#endif

#endif /* NF_IO_H */
