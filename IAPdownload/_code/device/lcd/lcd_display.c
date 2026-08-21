/**
 * @file    lcd_display.c
 * @brief   LCD显示模块 - 上半屏显示TX，下半屏显示RX
 * @details 简单实现：逐行追加显示，满屏后整体上滚一行
 */

#include "config.h"
#include "lcd_display.h"

/* 滚动文本缓冲区(存储最近N行的文本) */
static char tx_buf[LCD_MAX_TX_ROWS][LCD_MAX_COLS + 1];
static char rx_buf[LCD_MAX_RX_ROWS][LCD_MAX_COLS + 1];

static uint8_t tx_row = 0;     /* 当前写入行 */
static uint8_t tx_col = 0;     /* 当前列 */
static uint8_t rx_row = 0;
static uint8_t rx_col = 0;

/* 初始标记 */
static uint8_t disp_inited = 0;

/**
 * @brief       绘制分隔线
 */
static void lcd_draw_separator(void)
{
    lcd_fill(0, LCD_TX_AREA_Y + LCD_TX_AREA_H, LCD_DISP_WIDTH - 1, LCD_RX_AREA_Y - 1, BLUE);
}

/**
 * @brief       重绘TX区域所有行
 */
static void lcd_redraw_tx(void)
{
    uint8_t i;
    /* 清除TX区域 */
    lcd_fill(0, LCD_TX_AREA_Y, LCD_DISP_WIDTH - 1, LCD_TX_AREA_Y + LCD_TX_AREA_H - 1, WHITE);
    /* 逐行绘制 */
    for (i = 0; i < LCD_MAX_TX_ROWS; i++) {
        if (tx_buf[i][0] != '\0') {
            lcd_show_string(0, LCD_TX_AREA_Y + i * LCD_CHAR_H,
                            LCD_DISP_WIDTH, LCD_CHAR_H, 16, tx_buf[i], RED);
        }
    }
}

/**
 * @brief       重绘RX区域所有行
 */
static void lcd_redraw_rx(void)
{
    uint8_t i;
    lcd_fill(0, LCD_RX_AREA_Y, LCD_DISP_WIDTH - 1, LCD_RX_AREA_Y + LCD_RX_AREA_H - 1, WHITE);
    for (i = 0; i < LCD_MAX_RX_ROWS; i++) {
        if (rx_buf[i][0] != '\0') {
            lcd_show_string(0, LCD_RX_AREA_Y + i * LCD_CHAR_H,
                            LCD_DISP_WIDTH, LCD_CHAR_H, 16, rx_buf[i], BLUE);
        }
    }
}

/**
 * @brief       TX区域上滚一行
 */
static void tx_scroll_up(void)
{
    uint8_t i;
    for (i = 0; i < LCD_MAX_TX_ROWS - 1; i++) {
        memcpy(tx_buf[i], tx_buf[i + 1], LCD_MAX_COLS + 1);
    }
    tx_buf[LCD_MAX_TX_ROWS - 1][0] = '\0';
    tx_row = LCD_MAX_TX_ROWS - 1;
    tx_col = 0;
    lcd_redraw_tx();
}

/**
 * @brief       RX区域上滚一行
 */
static void rx_scroll_up(void)
{
    uint8_t i;
    for (i = 0; i < LCD_MAX_RX_ROWS - 1; i++) {
        memcpy(rx_buf[i], rx_buf[i + 1], LCD_MAX_COLS + 1);
    }
    rx_buf[LCD_MAX_RX_ROWS - 1][0] = '\0';
    rx_row = LCD_MAX_RX_ROWS - 1;
    rx_col = 0;
    lcd_redraw_rx();
}

/**
 * @brief       向缓冲区追加数据并刷新显示
 * @param       buf:   行缓冲区
 * @param       row:   当前行指针
 * @param       col:   当前列指针
 * @param       data:  数据
 * @param       len:   数据长度
 * @param       area_y: 区域起始Y坐标
 * @param       max_rows: 最大行数
 * @param       color: 文字颜色
 * @param       scroll_fn: 滚动函数
 */
static void append_data(char buf[][LCD_MAX_COLS + 1], uint8_t *row, uint8_t *col,
                         const uint8_t *data, uint32_t len,
                         uint16_t area_y, uint8_t max_rows, uint16_t color,
                         void (*scroll_fn)(void))
{
    uint32_t i;
    uint16_t px, py;

    for (i = 0; i < len; i++) {
        char ch = (char)data[i];

        /* 换行处理 */
        if (ch == '\n' || ch == '\r') {
            buf[*row][*col] = '\0';
            (*row)++;
            *col = 0;
            if (*row >= max_rows) {
                scroll_fn();
            }
            continue;
        }

        /* 可显示字符 */
        if (ch >= 0x20 && ch <= 0x7E) {
            /* 先在当前位置画一个空格覆盖旧字符 */
            px = (*col) * LCD_CHAR_W;
            py = area_y + (*row) * LCD_CHAR_H;
            lcd_show_char(px, py, ch, 16, 0, color);

            buf[*row][*col] = ch;
            (*col)++;

            /* 行满自动换行 */
            if (*col >= LCD_MAX_COLS) {
                buf[*row][*col] = '\0';
                (*row)++;
                *col = 0;
                if (*row >= max_rows) {
                    scroll_fn();
                }
            }
        }
    }
}

/* ==================== 异步刷新实现 ==================== */
static uint8_t tx_pending = 0;
static uint8_t rx_pending = 0;

static uint8_t tx_async_buf[LCD_BUF_SIZE];
static uint32_t tx_async_len = 0;

static uint8_t rx_async_buf[LCD_BUF_SIZE];
static uint32_t rx_async_len = 0;
/**
 * @brief       LCD显示处理主函数(分步状态机)
 * @note        在task_com1中周期调用，统一处理初始化和刷新
 *              状态: 0~4=初始化步骤, 5=正常刷新
 */
void LCD_Func(void)
{
    static uint8_t state = 0;
    
    switch(state) {
        /* ==================== 初始化阶段 ==================== */
        case 0:  /* 清屏 */
            lcd_clear(WHITE);
            state = 1;
            break;
            
        case 1:  /* 显示TX标题 */
            lcd_show_string(0, 0, LCD_DISP_WIDTH, LCD_CHAR_H, 16, (char*)"--- TX (PC->MCU) ---", RED);
            state = 2;
            break;
            
        case 2:  /* 画分隔线 */
            lcd_draw_separator();
            state = 3;
            break;
            
        case 3:  /* 显示RX标题 */
            lcd_show_string(0, LCD_RX_AREA_Y, LCD_DISP_WIDTH, LCD_CHAR_H, 16, (char*)"--- RX (MCU->PC) ---", BLUE);
            state = 4;
            break;
            
        case 4:  /* 初始化缓冲区 */
            memset(tx_buf, 0, sizeof(tx_buf));
            memset(rx_buf, 0, sizeof(rx_buf));
            tx_row = 1;  /* 跳过标题行 */
            tx_col = 0;
            rx_row = 1;
            rx_col = 0;
            disp_inited = 1;
            state = 5;  /* 进入正常刷新阶段 */
            break;
            
        /* ==================== 正常刷新阶段 ==================== */
        case 5:
            if (tx_pending) {
                LCD_Display_TX(tx_async_buf, tx_async_len);
                tx_pending = 0;
            }
            if (rx_pending) {
                LCD_Display_RX(rx_async_buf, rx_async_len);
                rx_pending = 0;
            }
            break;
            
        default:
            state = 0;
            break;
    }
}

/**
 * @brief       显示TX数据(USB CDC收到, PC发来的)
 */
void LCD_Display_TX(const uint8_t *data, uint32_t len)
{
    if (!disp_inited) return;
    append_data(tx_buf, &tx_row, &tx_col, data, len,
                LCD_TX_AREA_Y + LCD_CHAR_H, LCD_MAX_TX_ROWS, RED, tx_scroll_up);
}

/**
 * @brief       显示RX数据(UART收到, MCU发往PC的)
 */
void LCD_Display_RX(const uint8_t *data, uint32_t len)
{
    if (!disp_inited) return;
    append_data(rx_buf, &rx_row, &rx_col, data, len,
                LCD_RX_AREA_Y + LCD_CHAR_H, LCD_MAX_RX_ROWS, BLUE, rx_scroll_up);
}

/**
 * @brief       TX异步标记 - 在modefunc_Lastfunc中调用
 * @param       data: 数据指针
 * @param       len: 数据长度
 */
void LCD_Display_TX_Async(const uint8_t *data, uint32_t len)
{
    if (!disp_inited) return;
    if (len > LCD_BUF_SIZE) len = LCD_BUF_SIZE;
    memcpy(tx_async_buf, data, len);
    tx_async_len = len;
    tx_pending = 1;
}

/**
 * @brief       RX异步标记 - 在modefunc_Lastfunc中调用
 * @param       data: 数据指针
 * @param       len: 数据长度
 */
void LCD_Display_RX_Async(const uint8_t *data, uint32_t len)
{
    if (!disp_inited) return;
    if (len > LCD_BUF_SIZE) len = LCD_BUF_SIZE;
    memcpy(rx_async_buf, data, len);
    rx_async_len = len;
    rx_pending = 1;
}
