/* nf_console.c - UTF-8 控制台输出 */

#define _CRT_SECURE_NO_WARNINGS
#include "nf_console.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <windows.h>

static void nf_console_puts_utf8(const char* utf8) {
    if (!utf8) return;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD ft = GetFileType(hOut);
    /* 管道/重定向时（如被 Launcher 调用）：直接写 UTF-8 字节，便于前端正确显示 */
    if (ft == FILE_TYPE_PIPE || ft == FILE_TYPE_DISK) {
        size_t len = strlen(utf8);
        if (len > 0) {
            DWORD written;
            WriteFile(hOut, utf8, (DWORD)len, &written, NULL);
        }
        return;
    }
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    if (wlen <= 0) return;
    wchar_t* wbuf = (wchar_t*)malloc((size_t)wlen * sizeof(wchar_t));
    if (!wbuf) return;
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wbuf, wlen);
    DWORD written;
    WriteConsoleW(hOut, wbuf, (DWORD)(wlen - 1), &written, NULL);
    free(wbuf);
}

void nf_printf(const char* fmt, ...) {
    char buf[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    nf_console_puts_utf8(buf);
}
