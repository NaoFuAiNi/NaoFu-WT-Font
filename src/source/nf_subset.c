/* nf_subset.c - 调用 nf_subset_tool.exe */

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include "nf_subset.h"
#include "nf_console.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define NF_SUBSET_TOOL_NAME L"nf_subset_tool.exe"
#define CMD_BUF_WCHARS 2048
#define PATH_WCHARS 512
#define SUBSET_TIMEOUT_MS  (5 * 60 * 1000)  /* 子进程超时 5 分钟 */

/* UTF-8 转宽字符，写入 wbuf，最多 wbuf_count 个 wchar_t（含结尾）。返回 0 成功，-1 失败。 */
static int utf8_to_wide(const char* utf8, wchar_t* wbuf, size_t wbuf_count) {
    int n;
    if (!utf8 || !wbuf || wbuf_count == 0) return -1;
    n = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wbuf, (int)wbuf_count);
    if (n <= 0) return -1;
    return 0;
}

/* ref_path 可为 NULL：NULL 表示仅做西里尔→拉丁 remap，不瘦身；非 NULL 表示瘦身+remap。
 * 路径均为 UTF-8（含中文项目目录）。 */
s32 NF_runSubsetTool(const char* ref_path, const char* input_path, const char* output_path) {
    wchar_t exe_dir[MAX_PATH];
    wchar_t tool_path[MAX_PATH];
    wchar_t ref_wide[PATH_WCHARS];
    wchar_t input_wide[PATH_WCHARS];
    wchar_t output_wide[PATH_WCHARS];
    wchar_t cmd_line[CMD_BUF_WCHARS];
    STARTUPINFOW si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    DWORD exit_code = 1;
    size_t len;
    int n;

    if (!input_path || !output_path) return -1;
    si.cb = sizeof(si);

    if (GetModuleFileNameW(NULL, exe_dir, MAX_PATH) == 0) {
        nf_printf("[NaoFu] 无法获取主程序路径。\n");
        return -1;
    }
    len = wcslen(exe_dir);
    while (len > 0 && exe_dir[len - 1] != L'\\' && exe_dir[len - 1] != L'/') len--;
    exe_dir[len] = L'\0';
    swprintf_s(tool_path, MAX_PATH, L"%s%s", exe_dir, NF_SUBSET_TOOL_NAME);

    if (GetFileAttributesW(tool_path) == INVALID_FILE_ATTRIBUTES) {
        nf_printf("[NaoFu] 未找到瘦身工具: nf_subset_tool.exe\n");
        nf_printf("[NaoFu] 请将 nf_subset_tool.exe 放在主程序同一目录下。\n");
        return -1;
    }

    if (utf8_to_wide(input_path, input_wide, PATH_WCHARS) != 0 ||
        utf8_to_wide(output_path, output_wide, PATH_WCHARS) != 0) {
        nf_printf("[NaoFu] 路径转换失败。\n");
        return -1;
    }
    if (ref_path && ref_path[0] != '\0') {
        if (utf8_to_wide(ref_path, ref_wide, PATH_WCHARS) != 0) {
            nf_printf("[NaoFu] 路径转换失败。\n");
            return -1;
        }
        n = swprintf_s(cmd_line, CMD_BUF_WCHARS, L"\"%s\" \"%s\" \"%s\" \"%s\"",
                      tool_path, ref_wide, input_wide, output_wide);
    } else {
        n = swprintf_s(cmd_line, CMD_BUF_WCHARS, L"\"%s\" \"%s\" \"%s\"",
                      tool_path, input_wide, output_wide);
    }
    if (n < 0 || (size_t)n >= CMD_BUF_WCHARS) {
        nf_printf("[NaoFu] 命令行过长。\n");
        return -1;
    }

    /* 子进程工作目录必须为应用根目录，否则相对路径 font/custom/、font/source/、build/ 会解析到 tools 下导致找不到文件 */
    wchar_t work_dir[MAX_PATH];
    work_dir[0] = L'\0';
    if (GetCurrentDirectoryW(MAX_PATH, work_dir) == 0)
        wcscpy_s(work_dir, MAX_PATH, exe_dir);
    const wchar_t* child_cwd = (work_dir[0] != L'\0') ? work_dir : exe_dir;

    if (CreateProcessW(NULL, cmd_line, NULL, NULL, FALSE, 0, NULL, child_cwd, &si, &pi) == 0) {
        nf_printf("[NaoFu] 启动瘦身工具失败 (错误码 %lu)。\n", GetLastError());
        return -1;
    }
    if (WaitForSingleObject(pi.hProcess, SUBSET_TIMEOUT_MS) == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        nf_printf("[NaoFu] 瘦身工具执行超时（%u 分钟），已强制结束。请检查字体文件或重试。\n", (unsigned)(SUBSET_TIMEOUT_MS / 60000));
        return -1;
    }
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exit_code != 0) {
        nf_printf("[NaoFu] 瘦身工具执行失败 (退出码 %lu)。\n", exit_code);
        return -1;
    }
    return 0;
}
