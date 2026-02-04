/* main.c - 主程序入口 */

#define _CRT_SECURE_NO_WARNINGS
#include "nf_console.h"
#include "nf_fonts.h"
#include "nf_io.h"
#include "nf_patcher.h"
#include "nf_ui.h"
#include "nf_types.h"
#include "nf_version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <winsock2.h>
#include <windows.h>
#include <shellapi.h>
#include <iphlpapi.h>
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Iphlpapi.lib")

#define BUILD_DIR        "build"
#define PROJECT_DIR_MAX  80
#define PROJECT_NAME_MAX 64
#define AUTHOR_NAME_MAX  128
#define TOOL_VERSION     NF_TOOL_VERSION
/* built-in font pack: machine_id = this magic */
#define BUILTIN_MAGIC    "NAOFUBIN"
/* default author when empty (UTF-8) */
#define DEFAULT_AUTHOR   "\xe8\xbf\x99\xe4\xb8\xaa\xe7\x94\xa8\xe6\x88\xb7\xe5\xbe\x88\xe7\xa5\x9e\xe7\xa7\x98"

/* 完整覆盖：中英字符集，基本涵盖游戏内常见字符 */
static const u32 FULL_COVER_IDS[] = { 1, 2, 3, 4, 5, 9, 10, 13 };
#define FULL_COVER_COUNT  (sizeof(FULL_COVER_IDS) / sizeof(FULL_COVER_IDS[0]))
#define MAX_CHOICES       21

static size_t validate_project_name(char* name, size_t cap);

/* CLI parse result: cli_ok=1 means non-interactive. */
typedef struct {
    int cli_ok;
    char project_name[PROJECT_NAME_MAX + 1];
    char author_name[AUTHOR_NAME_MAX + 1];
    u32 mode;           /* 1/2/3 */
    u32 slots[MAX_CHOICES];
    u32 slot_count;     /* mode 2 时多个；mode 3 时 1 个放 slots[0] */
} CLIArgs_t;

static int write_data_naofu(const char *project_dir, const CLIArgs_t *cli);

/* 解析 --project / --mode / --slots 或 --slot，填充 *out。返回 1 表示走非交互。 */
static int parse_cli(int argc, char** argv, CLIArgs_t* out) {
    int i;
    const char* a;
    memset(out, 0, sizeof(CLIArgs_t));
    if (argc <= 1) return 0;
    for (i = 1; i < argc; i++) {
        a = argv[i];
        if (strcmp(a, "--project") == 0 && i + 1 < argc) {
            strncpy(out->project_name, argv[++i], PROJECT_NAME_MAX);
            out->project_name[PROJECT_NAME_MAX] = '\0';
        } else if (strcmp(a, "--mode") == 0 && i + 1 < argc) {
            out->mode = (u32)atoi(argv[++i]);
        } else if (strcmp(a, "--slots") == 0 && i + 1 < argc) {
            char buf[256];
            char* p;
            strncpy(buf, argv[++i], sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            for (p = strtok(buf, " ,\t"); p != NULL && out->slot_count < MAX_CHOICES; p = strtok(NULL, " ,\t")) {
                u32 num = (u32)atoi(p);
                if (num >= 1 && num <= g_num_font_choices)
                    out->slots[out->slot_count++] = num;
            }
        } else if (strcmp(a, "--slot") == 0 && i + 1 < argc) {
            u32 num = (u32)atoi(argv[++i]);
            if (num >= 1 && num <= g_num_font_choices) {
                out->slots[0] = num;
                out->slot_count = 1;
            }
        } else if (strcmp(a, "--author") == 0 && i + 1 < argc) {
            strncpy(out->author_name, argv[++i], AUTHOR_NAME_MAX);
            out->author_name[AUTHOR_NAME_MAX] = '\0';
        }
    }
    if (out->project_name[0] != '\0' && (out->mode == 1 || out->mode == 2 || out->mode == 3)) {
        if (out->mode == 2 && out->slot_count == 0) return 0; /* mode 2 必须带 --slots */
        if (out->mode == 3 && out->slot_count == 0) return 0;  /* mode 3 必须带 --slot */
        out->cli_ok = 1;
        return 1;
    }
    return 0;
}

/* 从宽字符命令行解析参数，项目名转为 UTF-8 存入 out。返回 1 表示走非交互。 */
static int parse_cli_wide(int argc_wide, wchar_t** argv_wide, CLIArgs_t* out) {
    int i;
    memset(out, 0, sizeof(CLIArgs_t));
    if (argc_wide <= 1 || !argv_wide) return 0;
    for (i = 1; i < argc_wide; i++) {
        if (wcscmp(argv_wide[i], L"--project") == 0 && i + 1 < argc_wide) {
            wchar_t* wname = argv_wide[++i];
            int n = WideCharToMultiByte(CP_UTF8, 0, wname, -1, out->project_name, (int)(PROJECT_NAME_MAX + 1), NULL, NULL);
            if (n <= 0) out->project_name[0] = '\0';
            else if (n > (int)(PROJECT_NAME_MAX + 1)) out->project_name[PROJECT_NAME_MAX] = '\0';
        } else if (wcscmp(argv_wide[i], L"--mode") == 0 && i + 1 < argc_wide) {
            out->mode = (u32)wcstoul(argv_wide[++i], NULL, 10);
        } else if (wcscmp(argv_wide[i], L"--slots") == 0 && i + 1 < argc_wide) {
            wchar_t* s = argv_wide[++i];
            for (; *s && out->slot_count < MAX_CHOICES; ) {
                u32 num = (u32)wcstoul(s, &s, 10);
                if (num >= 1 && num <= g_num_font_choices) out->slots[out->slot_count++] = num;
                while (*s == L' ' || *s == L',' || *s == L'\t') s++;
            }
        } else if (wcscmp(argv_wide[i], L"--slot") == 0 && i + 1 < argc_wide) {
            u32 num = (u32)wcstoul(argv_wide[++i], NULL, 10);
            if (num >= 1 && num <= g_num_font_choices) {
                out->slots[0] = num;
                out->slot_count = 1;
            }
        } else if (wcscmp(argv_wide[i], L"--author") == 0 && i + 1 < argc_wide) {
            wchar_t* wau = argv_wide[++i];
            int n = WideCharToMultiByte(CP_UTF8, 0, wau, -1, out->author_name, (int)(AUTHOR_NAME_MAX + 1), NULL, NULL);
            if (n <= 0) out->author_name[0] = '\0';
            else if (n > (int)(AUTHOR_NAME_MAX + 1)) out->author_name[AUTHOR_NAME_MAX] = '\0';
        }
    }
    if (out->project_name[0] != '\0' && (out->mode == 1 || out->mode == 2 || out->mode == 3)) {
        if (out->mode == 2 && out->slot_count == 0) return 0;
        if (out->mode == 3 && out->slot_count == 0) return 0;
        out->cli_ok = 1;
        return 1;
    }
    return 0;
}

/* 非交互模式：根据 CLI 参数创建项目目录并执行替换，所有输出走 stdout。成功返回 0，失败返回 1。 */
static int run_cli_mode(const CLIArgs_t* cli, char* project_dir, size_t project_dir_cap) {
    char name_buf[PROJECT_NAME_MAX + 2];
    size_t len;
    u32 idx;

    strncpy(name_buf, cli->project_name, sizeof(name_buf) - 1);
    name_buf[sizeof(name_buf) - 1] = '\0';
    len = validate_project_name(name_buf, sizeof(name_buf));
    if (len == 0) {
        nf_printf("[NaoFu] 错误：项目名无效（为空、过长或包含非法字符）。\n");
        return 1;
    }
    if ((size_t)snprintf(project_dir, project_dir_cap, "%s/%s", BUILD_DIR, name_buf) >= project_dir_cap) {
        nf_printf("[NaoFu] 错误：路径过长。\n");
        return 1;
    }
    NF_createDir(BUILD_DIR);
    if (NF_createDir(project_dir) != 0) {
        nf_printf("[NaoFu] 错误：无法创建目录 ");
        printf("%s\n", project_dir);
        return 1;
    }
    nf_printf("[NaoFu] 项目目录: ");
    printf("%s\n", project_dir);

    if (cli->mode == 1) {
        nf_printf("[NaoFu] 模式 1：完整覆盖，将依次替换序号 1,2,3,4,5,9,10,13\n");
        nf_printf("[NaoFu:PROGRESS] 0,%u\n", (unsigned)FULL_COVER_COUNT);
        for (idx = 0; idx < FULL_COVER_COUNT; idx++) {
            if (NF_runFontReplace(FULL_COVER_IDS[idx], project_dir, 1) != 0)
                return 1;
            nf_printf("[NaoFu:PROGRESS] %u,%u\n", (unsigned)(idx + 1), (unsigned)FULL_COVER_COUNT);
        }
    } else if (cli->mode == 2) {
        nf_printf("[NaoFu] 模式 2：自定义修改，共 %u 个字体槽位\n", cli->slot_count);
        nf_printf("[NaoFu:PROGRESS] 0,%u\n", cli->slot_count);
        for (idx = 0; idx < cli->slot_count; idx++) {
            if (NF_runFontReplace(cli->slots[idx], project_dir, 1) != 0)
                return 1;
            nf_printf("[NaoFu:PROGRESS] %u,%u\n", (unsigned)(idx + 1), cli->slot_count);
        }
    } else {
        nf_printf("[NaoFu] 模式 3：单个修改，替换序号 %u\n", cli->slots[0]);
        nf_printf("[NaoFu:PROGRESS] 0,1\n");
        if (NF_runFontReplace(cli->slots[0], project_dir, 0) != 0)
            return 1;
        nf_printf("[NaoFu:PROGRESS] 1,1\n");
    }
    if (write_data_naofu(project_dir, cli) != 0)
        return 1;
    /* 将完整字体复制到输出目录，供后续展示用（非裁切后的 slim） */
    {
        const char* full_font_src = "font/custom/MyFonts.ttf";
        char full_font_dest[512];
        NF_byte_t* buf = NULL;
        NF_file_size_t sz = 0;
        if (NF_pathExists(full_font_src) && NF_readFileToBuffer(full_font_src, &buf, &sz) == 0 && buf && sz > 0) {
            if ((size_t)snprintf(full_font_dest, sizeof(full_font_dest), "%s/MyFonts.ttf", project_dir) < sizeof(full_font_dest) &&
                NF_writeBufferToFile(full_font_dest, buf, sz) == 0)
                nf_printf("[NaoFu] 已复制完整字体到输出目录（供展示用）。\n");
            free(buf);
        }
    }
    nf_printf("[NaoFu] 全部完成。\n");
    return 0;
}

/* CRC32 表与计算（用于 DATA.NaoFu 校验） */
static u32 crc32_table[256];
static int crc32_table_done;

static void crc32_init(void) {
    u32 c;
    int n, k;
    for (n = 0; n < 256; n++) {
        c = (u32)n;
        for (k = 0; k < 8; k++) {
            if (c & 1) c = 0xEDB88320U ^ (c >> 1);
            else c = c >> 1;
        }
        crc32_table[n] = c;
    }
    crc32_table_done = 1;
}

static u32 crc32_append(u32 crc, const void* data, size_t len) {
    const unsigned char* p = (const unsigned char*)data;
    if (!crc32_table_done) crc32_init();
    crc = crc ^ 0xFFFFFFFFU;
    while (len--) crc = crc32_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFU;
}

#define MAX_MACS 32
/* 获取本机 machine_id 8 字节：收集所有非环回网卡 MAC(6 字节)，按字节序排序后取最小的 + 2 字节 0，与 C# 一致。 */
static void get_machine_id(NF_byte_t out[MACHINE_ID_SIZE]) {
    ULONG buf_len = 0;
    PIP_ADAPTER_ADDRESSES paddr = NULL;
    ULONG ret;
    NF_byte_t macs[MAX_MACS][6];
    size_t n_macs = 0;
    size_t i, j;
    memset(out, 0, MACHINE_ID_SIZE);
    ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &buf_len);
    if (ret != ERROR_BUFFER_OVERFLOW || buf_len == 0) return;
    paddr = (PIP_ADAPTER_ADDRESSES)malloc(buf_len);
    if (!paddr) return;
    ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, paddr, &buf_len);
    if (ret != NO_ERROR) { free(paddr); return; }
    for (; paddr && n_macs < MAX_MACS; paddr = paddr->Next) {
        if (paddr->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
        if (paddr->PhysicalAddressLength >= 6) {
            for (i = 0; i < 6; i++) macs[n_macs][i] = (NF_byte_t)paddr->PhysicalAddress[i];
            n_macs++;
        }
    }
    free(paddr);
    if (n_macs == 0) return;
    /* 按 6 字节字典序排序，取最小 */
    for (i = 0; i < n_macs - 1; i++) {
        for (j = i + 1; j < n_macs; j++) {
            int cmp = memcmp(macs[i], macs[j], 6);
            if (cmp > 0) {
                NF_byte_t t[6];
                memcpy(t, macs[i], 6);
                memcpy(macs[i], macs[j], 6);
                memcpy(macs[j], t, 6);
            }
        }
    }
    memcpy(out, macs[0], 6);
}

/* 在 project_dir 下写入 DATA.NaoFu：魔数、格式版本、时间戳、工具版本、项目名、作者名、machine_id(v2)、CRC32。 */
static int write_data_naofu(const char* project_dir, const CLIArgs_t* cli) {
    NF_byte_t buf[512];
    size_t off = 0;
    size_t plen, alen;
    const char* author_str;
    u32 crc;
    char path[512];

    if (!project_dir || !cli || sizeof(buf) < 64) return -1;

    memcpy(buf + off, DATA_NAOFU_MAGIC, 5);
    off += 5;
    buf[off++] = (NF_byte_t)DATA_NAOFU_VER;

    {
        time_t t = time(NULL);
        u32 ts = (u32)(t > 0 ? t : 0);
        buf[off++] = (NF_byte_t)(ts & 0xFF);
        buf[off++] = (NF_byte_t)((ts >> 8) & 0xFF);
        buf[off++] = (NF_byte_t)((ts >> 16) & 0xFF);
        buf[off++] = (NF_byte_t)((ts >> 24) & 0xFF);
    }

    plen = strlen(TOOL_VERSION);
    if (plen > 255) plen = 255;
    buf[off++] = (NF_byte_t)plen;
    memcpy(buf + off, TOOL_VERSION, plen);
    off += plen;

    plen = strlen(cli->project_name);
    if (plen > 255) plen = 255;
    buf[off++] = (NF_byte_t)plen;
    memcpy(buf + off, cli->project_name, plen);
    off += plen;

    author_str = (cli->author_name[0] != '\0') ? cli->author_name : DEFAULT_AUTHOR;
    alen = strlen(author_str);
    if (alen > 255) alen = 255;
    buf[off++] = (NF_byte_t)alen;
    memcpy(buf + off, author_str, alen);
    off += alen;

    /* v2: machine_id 8 bytes (self-made = this machine MAC) */
    if (off + MACHINE_ID_SIZE + 4 > sizeof(buf)) return -1;
    get_machine_id(buf + off);
    off += MACHINE_ID_SIZE;

    crc = crc32_append(0, buf, off);
    buf[off++] = (NF_byte_t)(crc & 0xFF);
    buf[off++] = (NF_byte_t)((crc >> 8) & 0xFF);
    buf[off++] = (NF_byte_t)((crc >> 16) & 0xFF);
    buf[off++] = (NF_byte_t)((crc >> 24) & 0xFF);

    if ((size_t)snprintf(path, sizeof(path), "%s/DATA.NaoFu", project_dir) >= sizeof(path))
        return -1;
    if (NF_writeBufferToFile(path, buf, (NF_file_size_t)off) != 0) {
        nf_printf("[NaoFu] 警告：无法写入 %s\n", path);
        return -1;
    }
    return 0;
}

/* 检测 build 目录下是否存在至少一个子目录（视为已有项目），排除 . 与 .. */
static int has_build_project_folders(void) {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    int found = 0;
    h = FindFirstFileA(BUILD_DIR "\\*", &fd);
    if (h == INVALID_HANDLE_VALUE)
        return 0;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;
        if (fd.cFileName[0] == '.' && (fd.cFileName[1] == '\0' || (fd.cFileName[1] == '.' && fd.cFileName[2] == '\0')))
            continue;
        found = 1;
        break;
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return found;
}

/* 去除首尾空格，返回合法长度；若全空、过长或含非法字符返回 0。非法字符: \ / : * ? " < > | */
static size_t validate_project_name(char* name, size_t cap) {
    size_t i, start = 0, end = 0;
    const char* bad = "\\/:*?\"<>|";
    while (start < cap && name[start] == ' ') start++;
    if (start >= cap || name[start] == '\0') return 0;
    end = start;
    while (end < cap && name[end] != '\0') {
        for (i = 0; bad[i]; i++)
            if (name[end] == bad[i]) return 0;
        end++;
    }
    while (end > start && name[end - 1] == ' ') end--;
    if (end - start == 0 || end - start > PROJECT_NAME_MAX) return 0;
    memmove(name, name + start, end - start + 1);
    name[end - start] = '\0';
    return (size_t)(end - start);
}

/* 让用户输入项目名并创建 build/<name>，写入 project_dir，成功返回 0 */
static int ask_and_create_project_dir(char* project_dir, size_t cap) {
    char buf[256];
    size_t len;
    for (;;) {
        nf_printf("[NaoFu] 请输入这次制作字体的名字（将用作文件夹名）: ");
        if (!fgets(buf, sizeof(buf), stdin)) {
            nf_printf("\n[NaoFu] 输入已取消。\n");
            return -1;
        }
        buf[sizeof(buf) - 1] = '\0';
        { char* p = strchr(buf, '\n'); if (p) *p = '\0'; }
        len = validate_project_name(buf, sizeof(buf));
        if (len == 0) {
            nf_printf("[NaoFu] 文件夹名无效（为空、过长或包含 \\ / : * ? \" < > |）。请重新输入。\n");
            continue;
        }
        if ((size_t)snprintf(project_dir, cap, "%s/%s", BUILD_DIR, buf) >= cap) {
            nf_printf("[NaoFu] 路径过长。\n");
            return -1;
        }
        CreateDirectoryA(BUILD_DIR, NULL);
        if (CreateDirectoryA(project_dir, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
            nf_printf("[NaoFu] 无法创建目录: ");
            printf("%s\n", project_dir);
            return -1;
        }
        nf_printf("[NaoFu] 已创建项目目录: ");
        printf("%s\n", project_dir);
        return 0;
    }
}

int main(int argc, char** argv) {
    u32 user_choice = 0;
    char ch;
    char project_dir[PROJECT_DIR_MAX];
    CLIArgs_t cli;

    setlocale(LC_ALL, "");

    /* 优先用宽字符命令行解析（支持 Launcher 传入的中文项目名），再尝试窄字符 */
    {
        int argc_wide = 0;
        wchar_t** argv_wide = CommandLineToArgvW(GetCommandLineW(), &argc_wide);
        if (argv_wide && parse_cli_wide(argc_wide, argv_wide, &cli)) {
            LocalFree(argv_wide);
            return run_cli_mode(&cli, project_dir, sizeof(project_dir)) != 0 ? 1 : 0;
        }
        if (argv_wide) LocalFree(argv_wide);
    }
    if (parse_cli(argc, argv, &cli)) {
        return run_cli_mode(&cli, project_dir, sizeof(project_dir)) != 0 ? 1 : 0;
    }

    /* 以下为交互模式 */
    /* 先显示标题+作者+分隔线，再问项目名，最后显示提示+字体列表并选字体 */
    NF_displayMenuHeader();

    /* 若 build 下已有项目文件夹，则先选择：1. 创建新的字体  2. 更改之前的字体（尚未完成） */
    if (has_build_project_folders()) {
        for (;;) {
            nf_printf("[NaoFu] 检测到已有项目文件夹。请选择：\n");
            nf_printf("  1. 创建新的字体\n");
            nf_printf("  2. 更改之前的字体（尚未完成）\n");
            nf_printf("[NaoFu] 请输入 1 或 2: ");
            if (scanf("%u", &user_choice) != 1) user_choice = 0;
            while (getchar() != '\n') { }
            if (user_choice == 1)
                break;
            if (user_choice == 2) {
                nf_printf("\n[NaoFu] 该功能尚未完成，请选择「创建新的字体」。\n");
                continue;
            }
            nf_printf("\n[NaoFu] 无效输入，请输入 1 或 2。\n");
        }
        if (ask_and_create_project_dir(project_dir, sizeof(project_dir)) != 0) {
            NF_pause();
            return 1;
        }
    } else {
        /* 第一次使用：直接让用户输入项目名并创建目录 */
        nf_printf("[NaoFu] 请为本次制作的字体起一个名字（将作为 build 下的文件夹名）。\n");
        if (ask_and_create_project_dir(project_dir, sizeof(project_dir)) != 0) {
            NF_pause();
            return 1;
        }
    }

    /* 选择模式：1. 完整覆盖  2. 自定义修改（一次输入多个序号）  3. 单个修改 */
    nf_printf("-----------------------------------------------------------------------\n");
    nf_printf("[NaoFu] 请选择模式：\n");
    nf_printf("  1. 完整覆盖（中英字符集，基本涵盖你看到的所有字符）\n");
    nf_printf("  2. 自定义修改（列出所有选项，一次性输入多个字体序号）\n");
    nf_printf("  3. 单个修改（选一个改一个，可多次继续）\n");
    nf_printf("[NaoFu] 请输入 1、2 或 3: ");
    {
        u32 mode = 0;
        if (scanf("%u", &mode) != 1) mode = 0;
        while (getchar() != '\n') { }
        if (mode != 1 && mode != 2 && mode != 3) {
            nf_printf("\n[NaoFu] 无效输入，请输入 1、2 或 3。\n");
            NF_pause();
            return 1;
        }

        if (mode == 1) {
            /* 完整覆盖：固定 1,2,3,4,5,9,10,13 依次替换 */
            u32 idx;
            nf_printf("\n[NaoFu] 将依次替换序号: 1, 2, 3, 4, 5, 9, 10, 13\n");
            for (idx = 0; idx < FULL_COVER_COUNT; idx++) {
                if (NF_runFontReplace(FULL_COVER_IDS[idx], project_dir, 1) != 0) {
                    NF_pause();
                    return 1;
                }
            }
        } else if (mode == 2) {
            /* 自定义修改：显示列表，一次性输入多个序号（空格或逗号分隔） */
            char buf[256];
            char* p;
            u32 choices[MAX_CHOICES];
            u32 n_choices = 0;
            u32 idx;
            NF_displayMenuList();
            nf_printf("[NaoFu] 请输入要替换的字体序号，多个用空格或逗号分隔（如 1 2 5 10）: ");
            if (!fgets(buf, sizeof(buf), stdin)) {
                nf_printf("\n[NaoFu] 输入已取消。\n");
                NF_pause();
                return 1;
            }
            buf[sizeof(buf) - 1] = '\0';
            for (p = strtok(buf, " ,\t\n"); p != NULL && n_choices < MAX_CHOICES; p = strtok(NULL, " ,\t\n")) {
                u32 num = (u32)atoi(p);
                if (num >= 1 && num <= g_num_font_choices)
                    choices[n_choices++] = num;
            }
            if (n_choices == 0) {
                nf_printf("[NaoFu] 未输入有效序号（1-%u），已跳过。\n", g_num_font_choices);
            } else {
                for (idx = 0; idx < n_choices; idx++) {
                    if (NF_runFontReplace(choices[idx], project_dir, 1) != 0) {
                        NF_pause();
                        return 1;
                    }
                }
            }
        } else {
            /* 单个修改：列出选项，选一个改一个，可继续 */
            NF_displayMenuList();
            for (;;) {
                nf_printf("[NaoFu] 请输入您想替换的字体序号 (1-%u): ", g_num_font_choices);
                if (scanf("%u", &user_choice) != 1) {
                    nf_printf("\n[NaoFu] 错误：无效的输入，请输入一个数字。\n");
                    while (getchar() != '\n') { }
                    NF_pause();
                    return 1;
                }
                while (getchar() != '\n') { }
                if (NF_runFontReplace(user_choice, project_dir, 0) != 0) {
                    NF_pause();
                    return 1;
                }
                nf_printf("\n[NaoFu] 是否继续替换其他字体？(y/n): ");
                if (scanf(" %c", &ch) != 1) ch = 'n';
                while (getchar() != '\n') { }
                if (ch != 'y' && ch != 'Y')
                    break;
                nf_printf("\n");
                NF_displayMenuList();
            }
        }
    }

    nf_printf("\n[NaoFu] 按任意键退出...\n");
    NF_pause();
    return 0;
}
