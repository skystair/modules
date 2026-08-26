#ifndef __UI_DISPLAY_H
#define __UI_DISPLAY_H

#include <stdint.h>

/**
 * @brief       UI初始化（绘制静态元素）
 * @note        在lcd_init完成后调用一次
 */
void ui_init(void);

/**
 * @brief       UI周期刷新
 * @note        在task_com1中调用，内部节流(200ms)
 */
void ui_refresh(void);

/**
 * @brief       强制全屏重绘
 */
void ui_full_redraw(void);

#endif
