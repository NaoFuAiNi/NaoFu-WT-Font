/* nf_console.h - UTF-8 控制台输出 */

#ifndef NF_CONSOLE_H
#define NF_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* 按 UTF-8 格式化输出到控制台（类似 printf，不依赖代码页） */
void nf_printf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* NF_CONSOLE_H */
