#ifndef __LCD_DISPLAY_H
#define __LCD_DISPLAY_H

#include <stdint.h>

/* LCD显示常量(竖屏320x480, ST7789) */
#define LCD_DISP_WIDTH      320
#define LCD_DISP_HEIGHT     480
#define LCD_CHAR_W          8   /* 16号字体宽8像素 */
#define LCD_CHAR_H          16  /* 16号字体高16像素 */
#define LCD_MAX_COLS        (LCD_DISP_WIDTH / LCD_CHAR_W)    /* 每行最大字符数=40 */
#define LCD_MAX_ROWS        (LCD_DISP_HEIGHT / LCD_CHAR_H)   /* 行数=30 */

/* LCD主处理函数(已弃用，由ui_refresh替代) */
void LCD_Func(void);

#endif
