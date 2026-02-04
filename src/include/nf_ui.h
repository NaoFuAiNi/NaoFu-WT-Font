/* nf_ui.h - 控制台菜单 */

#ifndef NF_UI_H
#define NF_UI_H

#ifdef __cplusplus
extern "C" {
#endif

void NF_displayMenuHeader(void);   /* 标题 + 作者 + 分隔线 */
void NF_displayMenuList(void);    /* 提示 + 字体列表 */
void NF_displayMenu(void);        /* 上面两者合在一起 */
void NF_pause(void);

#ifdef __cplusplus
}
#endif

#endif /* NF_UI_H */
