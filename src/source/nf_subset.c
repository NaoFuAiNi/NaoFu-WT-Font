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
#define PIPE_READ_BUF_SIZE 8192
#define PIPE_OUTPUT_MAX    16384

/* 从管道读取全部内容，追加到 out_buf（最多 out_max 字节，含结尾 \0），返回写入长度（不含 \0）。 */
static size_t read_pipe_into(HANDLE hPipe, char* out_buf, size_t out_max) {
    char tmp[512];
    DWORD read_len = 0;
    size_t total = 0;
    if (!out_buf || out_max == 0) return 0;
    out_buf[0] = '\0';
    while (total + 1 < out_max && ReadFile(hPipe, tmp, sizeof(tmp), &read_len, NULL) && read_len > 0) {
        size_t copy = read_len;
        if (total + copy + 1 > out_max) copy = out_max - total - 1;
        memcpy(out_buf + total, tmp, copy);
        total += copy;
        out_buf[total] = '\0';
    }
    return total;
}

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
    swprintf_s(tool_path, MAX_PATH, L"%s\\%s", exe_dir, NF_SUBSET_TOOL_NAME);

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

    /* 管道抓子进程输出，方便看 Python 报错 */
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE hPipeRead = NULL, hPipeWrite = NULL;
    HANDLE hStdInNul = INVALID_HANDLE_VALUE;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    if (CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0)) {
        SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0);
        hStdInNul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = (hStdInNul != INVALID_HANDLE_VALUE) ? hStdInNul : GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = hPipeWrite;
        si.hStdError = hPipeWrite;
    }

    if (CreateProcessW(NULL, cmd_line, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, child_cwd, &si, &pi) == 0) {
        nf_printf("[NaoFu] 启动瘦身工具失败 (错误码 %lu)。\n", GetLastError());
        if (hPipeRead) CloseHandle(hPipeRead);
        if (hPipeWrite) CloseHandle(hPipeWrite);
        if (hStdInNul != INVALID_HANDLE_VALUE) CloseHandle(hStdInNul);
        return -1;
    }
    if (hPipeWrite) { CloseHandle(hPipeWrite); hPipeWrite = NULL; }
    if (hStdInNul != INVALID_HANDLE_VALUE) { CloseHandle(hStdInNul); hStdInNul = INVALID_HANDLE_VALUE; }

    if (WaitForSingleObject(pi.hProcess, SUBSET_TIMEOUT_MS) == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if (hPipeRead) CloseHandle(hPipeRead);
        nf_printf("[NaoFu] 瘦身工具执行超时（%u 分钟），已强制结束。请检查字体文件或重试。\n", (unsigned)(SUBSET_TIMEOUT_MS / 60000));
        return -1;
    }
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    char tool_output[PIPE_OUTPUT_MAX];
    size_t out_len = 0;
    tool_output[0] = '\0';
    if (hPipeRead) {
        out_len = read_pipe_into(hPipeRead, tool_output, sizeof(tool_output));
        CloseHandle(hPipeRead);
        if (out_len > 0) {
            nf_printf("[NaoFu] 瘦身工具输出:\n");
            nf_printf("%s\n", tool_output);
        }
    }

    /* 失败时始终写日志：优先写当前工作目录（Launcher 的 net8.0-windows），失败再试 exe 所在目录 */
    if (exit_code != 0) {
        wchar_t log_path[MAX_PATH];
        const wchar_t* log_dir = (work_dir[0] != L'\0') ? work_dir : exe_dir;
        swprintf_s(log_path, MAX_PATH, L"%s\\nf_subset_tool_log.txt", log_dir);
        HANDLE hLog = CreateFileW(log_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hLog == INVALID_HANDLE_VALUE && log_dir != exe_dir) {
            swprintf_s(log_path, MAX_PATH, L"%s\\nf_subset_tool_log.txt", exe_dir);
            hLog = CreateFileW(log_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        if (hLog != INVALID_HANDLE_VALUE) {
            DWORD written;
            if (out_len > 0)
                WriteFile(hLog, tool_output, (DWORD)out_len, &written, NULL);
            else {
                const char* fallback = "(瘦身工具退出码 1，未捕获到标准输出，可能为字体不兼容或 Python 环境异常。请尝试使用内置字体或联系作者。)";
                WriteFile(hLog, fallback, (DWORD)strlen(fallback), &written, NULL);
            }
            CloseHandle(hLog);
        } else
            nf_printf("[NaoFu] 无法创建 nf_subset_tool_log.txt (错误码 %lu)\n", GetLastError());
    }

    if (exit_code != 0) {
        /* 把详细错误打到 stdout，Launcher 会捕获并显示在界面，不依赖日志文件 */
        if (out_len > 0) {
            nf_printf("[NaoFu] 瘦身工具详细输出: %s\n", tool_output);
        } else {
            nf_printf("[NaoFu] 瘦身工具详细: 未捕获到输出，可能为字体不兼容或 Python 环境异常，请尝试内置字体。\n");
        }
        nf_printf("[NaoFu] 瘦身工具执行失败 (退出码 %lu)。\n", exit_code);
        return -1;
    }
    return 0;
}
