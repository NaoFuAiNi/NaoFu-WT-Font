/* nf_io.c - 文件读写，路径 UTF-8 */

#define _CRT_SECURE_NO_WARNINGS
#include "nf_io.h"
#include "nf_console.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define UTF8_WBUF_MAX 2048

#ifdef _WIN32
/* UTF-8 转宽字符，路径中 / 转为 \\。返回 0 成功，-1 失败。 */
static int utf8_to_wide(const char* utf8, wchar_t* wbuf, size_t wbuf_count) {
    int n;
    wchar_t* p;
    if (!utf8 || !wbuf || wbuf_count == 0) return -1;
    n = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wbuf, (int)wbuf_count);
    if (n <= 0) return -1;
    for (p = wbuf; *p; p++) {
        if (*p == L'/') *p = L'\\';
    }
    return 0;
}
#endif

s32 NF_readFileToBuffer(const char* filename, NF_byte_t** buffer_ptr, NF_file_size_t* file_size_ptr) {
    NF_file_handle_t fp = NULL;
#ifdef _WIN32
    wchar_t wpath[UTF8_WBUF_MAX];
    if (utf8_to_wide(filename, wpath, UTF8_WBUF_MAX) == 0) {
        fp = _wfopen(wpath, L"rb");
    }
    if (fp == NULL) {
        errno_t err = fopen_s(&fp, filename, "rb");
        (void)err;
    }
#else
    errno_t err = fopen_s(&fp, filename, "rb");
    (void)err;
#endif
    if (fp == NULL) {
        nf_printf("\n[NaoFu] 文件错误：无法打开文件 '%s'。\n", filename);
        nf_printf("[NaoFu] 请确保该文件存在于正确的路径下，并且没有被其他程序占用。\n");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    *file_size_ptr = ftell(fp);
    rewind(fp);
    if (*file_size_ptr <= 0) {
        nf_printf("\n[NaoFu] 文件错误：文件 '%s' 为空或读取大小失败。\n", filename);
        fclose(fp);
        return -1;
    }
    *buffer_ptr = (NF_byte_t*)malloc(*file_size_ptr);
    if (*buffer_ptr == NULL) {
        nf_printf("\n[NaoFu] 内存错误：无法为文件 '%s' 分配 %ld 字节的内存。\n", filename, *file_size_ptr);
        fclose(fp);
        return -1;
    }
    size_t bytes_read = fread(*buffer_ptr, 1, *file_size_ptr, fp);
    if (bytes_read != (size_t)*file_size_ptr) {
        nf_printf("\n[NaoFu] 文件错误：读取文件 '%s' 时发生错误。\n", filename);
        fclose(fp);
        free(*buffer_ptr);
        *buffer_ptr = NULL;
        return -1;
    }
    fclose(fp);
    return 0;
}

s32 NF_writeBufferToFile(const char* filename, const NF_byte_t* buffer, NF_file_size_t size) {
#ifdef _WIN32
    wchar_t wpath[UTF8_WBUF_MAX];
    FILE* fp = NULL;
    if (utf8_to_wide(filename, wpath, UTF8_WBUF_MAX) == 0) {
        fp = _wfopen(wpath, L"wb");
    }
    if (fp == NULL) {
        errno_t err;
        err = fopen_s(&fp, filename, "wb");
        if (err != 0 || fp == NULL) return -1;
    }
#else
    FILE* fp = NULL;
    errno_t err = fopen_s(&fp, filename, "wb");
    if (err != 0 || fp == NULL) return -1;
#endif
    if (buffer && size > 0) {
        if (fwrite(buffer, 1, (size_t)size, fp) != (size_t)size) {
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);
    return 0;
}

int NF_pathExists(const char* path_utf8) {
#ifdef _WIN32
    wchar_t wpath[UTF8_WBUF_MAX];
    if (utf8_to_wide(path_utf8, wpath, UTF8_WBUF_MAX) != 0) return 0;
    return (GetFileAttributesW(wpath) != INVALID_FILE_ATTRIBUTES) ? 1 : 0;
#else
    (void)path_utf8;
    return 0;
#endif
}

s32 NF_createDir(const char* path_utf8) {
#ifdef _WIN32
    wchar_t wpath[UTF8_WBUF_MAX];
    if (utf8_to_wide(path_utf8, wpath, UTF8_WBUF_MAX) != 0) return -1;
    if (CreateDirectoryW(wpath, NULL) != 0) return 0;
    return (GetLastError() == ERROR_ALREADY_EXISTS) ? 0 : -1;  /* 已存在视为成功 */
#else
    (void)path_utf8;
    return -1;
#endif
}
