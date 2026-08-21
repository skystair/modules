#ifndef __LCD_DISPLAY_H
#define __LCD_DISPLAY_H

#include <stdint.h>

/* LCD显示区域划分(竖屏240x320) */
/* LCD刷新缓冲区大小 */
#define LCD_BUF_SIZE        64
#define LCD_DISP_WIDTH      240
#define LCD_DISP_HEIGHT     320
#define LCD_CHAR_W          8   /* 16号字体宽8像素 */
#define LCD_CHAR_H          16  /* 16号字体高16像素 */

#define LCD_TX_AREA_Y       0       /* TX区域起始Y */
#define LCD_TX_AREA_H       152     /* TX区域高度(留8像素分隔线) */
#define LCD_RX_AREA_Y       160     /* RX区域起始Y */
#define LCD_RX_AREA_H       152     /* RX区域高度 */

#define LCD_MAX_COLS        (LCD_DISP_WIDTH / LCD_CHAR_W)    /* 每行最大字符数=30 */
#define LCD_MAX_TX_ROWS     (LCD_TX_AREA_H / LCD_CHAR_H)     /* TX行数=9 */
#define LCD_MAX_RX_ROWS     (LCD_RX_AREA_H / LCD_CHAR_H)     /* RX行数=9 */

/* LCD主处理函数 - 在task_com1中调用，统一处理初始化和刷新 */
void LCD_Func(void);

/* 显示TX数据(PC→MCU, 即USB CDC收到的数据) */
void LCD_Display_TX(const uint8_t *data, uint32_t len);

/* 显示RX数据(MCU→PC, 即UART收到的数据) */
void LCD_Display_RX(const uint8_t *data, uint32_t len);

/* 异步刷新接口 - 在modefunc_Lastfunc中调用 */
void LCD_Display_TX_Async(const uint8_t *data, uint32_t len);
void LCD_Display_RX_Async(const uint8_t *data, uint32_t len);

#endif
