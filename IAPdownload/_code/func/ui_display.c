/**
 * @file    ui_display.c
 * @brief   IAP Downloader UI显示模块
 * @details 320x480竖屏(ST7789), 8x16字体, 40列x30行
 *          深色背景 + 亮色文字主题
 *
 *  全屏布局 (40×30):
 *  Row 0:  "==== IAP Downloader v1.0 ===="     标题
 *  Row 1:  "USB:OK   Slots:2/5   FW:v1.0"     设备信息
 *  Row 2:  "========================================" 分隔
 *  Row 3:  " #  File Name              Size  CRC" 表头
 *  Row 4~8: ">0  firmware.bin          32kB  A1B2" 槽位S0~S4
 *  Row 9:  "========================================" 分隔
 *  Row 10: "Status: Idle"                       状态
 *  Row 11: "[=========................]"         进度条
 *  Row 12: "Recv: 0 / 0 B    Speed: ---"       传输数据
 *  Row 13: "CRC32: --------"                    CRC校验
 *  Row 14: "========================================" 分隔
 *  Row 15: "=== Commands ==="                   命令标题
 *  Row 16: "[Q] Query    [S] Select  [T] Xfer"  命令1
 *  Row 17: "[V] Verify   [D] Delete  [E] Erase" 命令2
 *  Row 18: "[I] Info     [F] Format"            命令3
 *  Row 19: "----------------------------------------" 分隔
 *  Row 20: "Message: Transfer OK!"              命令反馈
 *  Row 21: "========================================" 分隔
 *  Row 22: "Remote:  [I] Info   [F] Format"     远程命令
 *  Row 23: "Slot:    [U] Up     [D] Down"       槽位选择
 *  Row 24: "File:    [R] Refresh [X] Transfer"  文件操作
 *  Row 25: "========================================" 分隔
 *  Row 26: "[==== Progress ====] 45%"           进度2
 *  Row 27: "Transfer: firmware.bin -> S0"       传输详情
 *  Row 28: "Speed: 12.5 KB/s    ETA: 00:05"    速度/ETA
 *  Row 29: "IAP Downloader v1.0 by STM32F103"  版本
 */

#include "config.h"
#include "lcd.h"

/* ==================== 布局常量 ==================== */
#define MAX_COL         LCD_MAX_COLS    /* 40列 */

/* 行号定义 */
#define ROW_TITLE       0
#define ROW_INFO        1
#define ROW_SEP1        2
#define ROW_SLOT_HDR    3
#define ROW_SLOT_BASE   4       /* S0~S4 = Row 4~8 */
#define ROW_SEP2        9
#define ROW_STATUS      10
#define ROW_PROGRESS    11
#define ROW_DATA        12
#define ROW_CRC         13
#define ROW_SEP3        14
#define ROW_CMD_TITLE   15
#define ROW_CMD_LINE1   16
#define ROW_CMD_LINE2   17
#define ROW_CMD_LINE3   18
#define ROW_SEP4        19
#define ROW_CMD_MSG     20
#define ROW_SEP5        21
#define ROW_REMOTE      22
#define ROW_SLOT_SEL    23
#define ROW_FILE_OP     24
#define ROW_SEP6        25
#define ROW_PROGRESS2   26
#define ROW_XFER_FILE   27
#define ROW_SPEED       28
#define ROW_VERSION     29

/* 深色背景配色 */
#define CLR_BG          0x0010  /* 深蓝黑背景 */
#define CLR_TITLE       0x07FF  /* 青色标题 */
#define CLR_INFO        0xC618  /* 浅灰色信息 */
#define CLR_SEP         0x4208  /* 深灰分隔线 */
#define CLR_SLOT_HDR    0x07FF  /* 青色表头 */
#define CLR_SLOT_FILE   0xFFFF  /* 白色文件名 */
#define CLR_SLOT_EMPTY  0x8410  /* 灰色空槽 */
#define CLR_SELECT      0xFFE0  /* 黄色选中指示符 */
#define CLR_CMD_OK      0x07E0  /* 绿色成功 */
#define CLR_CMD_ERR     0xF800  /* 红色错误 */
#define CLR_CMD_INFO    0xFFE0  /* 黄色信息 */
#define CLR_BAR_FILL    0x07E0  /* 绿色进度 */
#define CLR_BAR_EMPTY   0x4208  /* 深灰空白 */
#define CLR_ROW_BG      0x0010  /* 行背景 */

/* ==================== 内部状态 ==================== */
static uint8_t  g_inited = 0;

/* UI状态快照 */
static int8_t   g_selected_slot = 0;    /* 当前选中槽位 (UI光标) */
static int8_t   g_last_xfer_state = -1;
static uint8_t  g_slots_dirty = 1;      /* 槽位列表需要重绘 */

/* 命令反馈消息 */
static char     g_cmd_msg[40] = "";
static uint16_t g_cmd_color = CLR_CMD_INFO;
static uint8_t  g_cmd_msg_valid = 0;

/* 刷新计时 */
static uint32_t g_last_refresh_tick = 0;
#define REFRESH_INTERVAL_MS     500

/* ==================== 工具函数 ==================== */

/**
 * @brief       在指定行显示文本（左对齐，填充空格到40列）
 */
static void ui_print_line(uint8_t row, const char *text, uint16_t color)
{
    uint16_t y = row * LCD_CHAR_H;
    char buf[MAX_COL + 1];
    uint8_t len = 0;

    while (text[len] && len < MAX_COL) {
        buf[len] = text[len];
        len++;
    }
    while (len < MAX_COL) {
        buf[len++] = ' ';
    }
    buf[MAX_COL] = '\0';

    lcd_fill(0, y, LCD_DISP_WIDTH - 1, y + LCD_CHAR_H - 1, CLR_ROW_BG);
    g_back_color = CLR_ROW_BG;  /* 设置字符背景色，避免lcd_show_char用白色填充 */
    lcd_show_string(0, y, LCD_DISP_WIDTH, LCD_CHAR_H, 16, buf, color);
}

/**
 * @brief       显示进度条 [=====....]
 */
static void ui_draw_progress(uint8_t row, uint8_t pct)
{
    uint16_t y = row * LCD_CHAR_H;
    char bar[MAX_COL + 1];
    uint8_t i;

    if (pct > 100) pct = 100;

    uint8_t inner = MAX_COL - 2;    /* 去掉 [ ] */
    uint8_t filled = (uint8_t)((uint16_t)pct * inner / 100);

    bar[0] = '[';
    for (i = 0; i < inner; i++) {
        bar[1 + i] = (i < filled) ? '=' : '.';
    }
    bar[MAX_COL - 1] = ']';
    bar[MAX_COL] = '\0';

    lcd_fill(0, y, LCD_DISP_WIDTH - 1, y + LCD_CHAR_H - 1, CLR_ROW_BG);
    g_back_color = CLR_ROW_BG;
    lcd_show_string(0, y, LCD_DISP_WIDTH, LCD_CHAR_H, 16, bar,
                    pct > 0 ? CLR_BAR_FILL : CLR_BAR_EMPTY);
}

/**
 * @brief       获取状态文本
 */
static const char *get_state_text(transfer_state_t st)
{
    switch (st) {
        case TRANSFER_IDLE:      return "Idle";
        case TRANSFER_WAIT_CMD:  return "Wait CMD";
        case TRANSFER_SELECTED:  return "Slot Selected";
        case TRANSFER_RECEIVING: return "Receiving XMODEM...";
        case TRANSFER_DONE:      return "Transfer OK!";
        case TRANSFER_ERROR:     return "Transfer Failed!";
        default:                 return "---";
    }
}

static uint16_t get_state_color(transfer_state_t st)
{
    switch (st) {
        case TRANSFER_DONE:      return CLR_CMD_OK;
        case TRANSFER_ERROR:     return CLR_CMD_ERR;
        case TRANSFER_SELECTED:  return CLR_CMD_OK;
        case TRANSFER_RECEIVING: return CLR_CMD_INFO;
        default:                 return CLR_CMD_INFO;
    }
}

/* ==================== 前向声明 ==================== */
static void ui_refresh_slot_line(uint8_t idx, uint8_t selected);

/* ==================== 按键处理 ==================== */

/**
 * @brief       处理按键输入
 */
static void ui_handle_keys(void)
{
    uint8_t up   = key_event_read(IO_INch_KEY_UP);
    uint8_t down = key_event_read(IO_INch_KEY_DOWN);

    if (up && g_selected_slot > 0) {
        /* 取消旧行高亮 */
        ui_refresh_slot_line(g_selected_slot, 0);
        g_selected_slot--;
        /* 高亮新行 */
        ui_refresh_slot_line(g_selected_slot, 1);
    }
    if (down && g_selected_slot < SLOT_COUNT - 1) {
        ui_refresh_slot_line(g_selected_slot, 0);
        g_selected_slot++;
        ui_refresh_slot_line(g_selected_slot, 1);
    }
}

/* ==================== 刷新子模块 ==================== */

/**
 * @brief       刷新单个槽位行
 * @param       idx: 槽位 0~4
 * @param       selected: 1=高亮选中, 0=普通
 */
static void ui_refresh_slot_line(uint8_t idx, uint8_t selected)
{
    file_info_t info;
    char line[MAX_COL + 1];
    uint8_t row = ROW_SLOT_BASE + idx;
    int ret;

    ret = flash_store_get_info(idx, &info);
    if (ret == FLASH_STORE_OK && info.size > 0) {
        char name_buf[MAX_FILE_NAME_LEN];
        strncpy(name_buf, info.name, sizeof(name_buf) - 1);
        name_buf[sizeof(name_buf) - 1] = '\0';

        if (selected) {
            snprintf(line, sizeof(line), ">%d %-20s %4ukB",
                     idx, name_buf, (unsigned)(info.size / 1024));
        } else {
            snprintf(line, sizeof(line), " %d %-20s %4ukB",
                     idx, name_buf, (unsigned)(info.size / 1024));
        }
        ui_print_line(row, line, selected ? CLR_SELECT : CLR_SLOT_FILE);
    } else {
        if (selected) {
            snprintf(line, sizeof(line), ">%d (Empty)", idx);
        } else {
            snprintf(line, sizeof(line), " %d (Empty)", idx);
        }
        ui_print_line(row, line, selected ? CLR_SELECT : CLR_SLOT_EMPTY);
    }
}

/**
 * @brief       刷新所有槽位
 */
static void ui_refresh_slots(void)
{
    uint8_t i;
    for (i = 0; i < SLOT_COUNT; i++) {
        ui_refresh_slot_line(i, (i == g_selected_slot) ? 1 : 0);
    }
}

/**
 * @brief       刷新状态行
 */
static void ui_refresh_status(void)
{
    char line[MAX_COL + 1];
    transfer_state_t st = transfer_get_state();
    int slot = transfer_get_slot();
    int count = flash_store_get_count();

    snprintf(line, sizeof(line), "Status: %s  Slot:%d/%d",
             get_state_text(st), slot, count);
    ui_print_line(ROW_STATUS, line, get_state_color(st));
}

/**
 * @brief       刷新信息行 (Row 1)
 */
static void ui_refresh_info(void)
{
    char line[MAX_COL + 1];
    int count = flash_store_get_count();

    snprintf(line, sizeof(line), "USB:OK   Slots:%d/%d   FW:v1.0",
             count, (int)SLOT_COUNT);
    ui_print_line(ROW_INFO, line, CLR_INFO);
}

/* ==================== 状态回调 ==================== */

static void on_transfer_state(transfer_state_t state, int progress)
{
    (void)progress;

    switch (state) {
        case TRANSFER_SELECTED:
            snprintf(g_cmd_msg, sizeof(g_cmd_msg), "Slot %d Selected", transfer_get_slot());
            g_cmd_color = CLR_CMD_OK;
            g_cmd_msg_valid = 1;
            break;
        case TRANSFER_RECEIVING:
            snprintf(g_cmd_msg, sizeof(g_cmd_msg), "Receiving XMODEM...");
            g_cmd_color = CLR_CMD_INFO;
            g_cmd_msg_valid = 1;
            break;
        case TRANSFER_DONE:
            snprintf(g_cmd_msg, sizeof(g_cmd_msg), "Transfer OK!");
            g_cmd_color = CLR_CMD_OK;
            g_cmd_msg_valid = 1;
            g_slots_dirty = 1;
            break;
        case TRANSFER_ERROR:
            snprintf(g_cmd_msg, sizeof(g_cmd_msg), "Transfer Failed!");
            g_cmd_color = CLR_CMD_ERR;
            g_cmd_msg_valid = 1;
            break;
        case TRANSFER_DELETED:
            snprintf(g_cmd_msg, sizeof(g_cmd_msg), "Deleted!");
            g_cmd_color = CLR_CMD_OK;
            g_cmd_msg_valid = 1;
            g_slots_dirty = 1;
            break;
        default:
            break;
    }
}

/* ==================== 公共API ==================== */

void ui_init(void)
{
    /* 全屏填充深色背景 */
    lcd_clear(CLR_BG);
    /* ---- 标题区 ---- */
    ui_print_line(ROW_TITLE,  "==== IAP Downloader v1.0 ====", CLR_TITLE);

    /* ---- 信息区 ---- */
    ui_refresh_info();

    /* ---- 槽位区 ---- */
    ui_print_line(ROW_SEP1,     "========================================", CLR_SEP);
    ui_print_line(ROW_SLOT_HDR, " #  File Name              Size  CRC",  CLR_SLOT_HDR);
    ui_refresh_slots();
    ui_print_line(ROW_SEP2,     "========================================", CLR_SEP);

    /* ---- 状态区 ---- */
    ui_refresh_status();
    ui_draw_progress(ROW_PROGRESS, 0);
    ui_print_line(ROW_DATA, "Recv: 0 / 0 B    Speed: ---",       CLR_INFO);
    ui_print_line(ROW_CRC,  "CRC32: --------",                    CLR_INFO);

    /* ---- 命令区 ---- */
    ui_print_line(ROW_SEP3,      "========================================",   CLR_SEP);
    ui_print_line(ROW_CMD_TITLE, "=== Commands ===",                             CLR_TITLE);
    ui_print_line(ROW_CMD_LINE1, "[Q] Query    [S] Select  [T] Xfer",           CLR_INFO);
    ui_print_line(ROW_CMD_LINE2, "[V] Verify   [D] Delete  [E] Erase",          CLR_INFO);
    ui_print_line(ROW_CMD_LINE3, "[I] Info     [F] Format",                      CLR_INFO);
    ui_print_line(ROW_SEP4,      "----------------------------------------",    CLR_SEP);
    ui_print_line(ROW_CMD_MSG,   "Message: ---",                                 CLR_CMD_INFO);
    ui_print_line(ROW_SEP5,      "========================================",    CLR_SEP);

    /* ---- 远程命令区 ---- */
    ui_print_line(ROW_REMOTE,  "Remote:  [I] Info   [F] Format",                CLR_INFO);
    ui_print_line(ROW_SLOT_SEL,"Slot:    [U] Up     [D] Down",                  CLR_INFO);
    ui_print_line(ROW_FILE_OP, "File:    [R] Refresh [X] Transfer",             CLR_INFO);
    ui_print_line(ROW_SEP6,    "========================================",       CLR_SEP);

    /* ---- 传输详情区 ---- */
    ui_draw_progress(ROW_PROGRESS2, 0);
    ui_print_line(ROW_XFER_FILE, "Transfer: ---",                                CLR_INFO);
    ui_print_line(ROW_SPEED,     "Speed: --- KB/s    ETA: --:--",                CLR_INFO);

    /* ---- 版本行 ---- */
    ui_print_line(ROW_VERSION, "IAP Downloader v1.0 by STM32F103",               CLR_SEP);

    /* 注册传输回调 */
    transfer_set_callback(on_transfer_state);

    g_inited = 1;
    g_last_xfer_state = -1;
    g_last_refresh_tick = osKernelGetTickCount();
}

void ui_full_redraw(void)
{
    g_inited = 0;
    g_slots_dirty = 1;
    ui_init();
}

void ui_refresh(void)
{
    transfer_state_t st;
    uint32_t now;

    if (!g_inited) return;

    /* 节流: 每500ms刷新一次 */
    now = osKernelGetTickCount();
    if ((now - g_last_refresh_tick) < REFRESH_INTERVAL_MS) {
        /* 按键处理不节流 */
        ui_handle_keys();
        return;
    }
    g_last_refresh_tick = now;

    /* 处理按键 */
    ui_handle_keys();

    /* 检查传输状态变化 */
    st = transfer_get_state();
    if ((int8_t)st != g_last_xfer_state) {
        g_last_xfer_state = (int8_t)st;
        ui_refresh_status();

        /* 状态变化时刷新信息行(文件数量可能变了) */
        ui_refresh_info();
    }

    /* 传输中: 刷新进度 */
    if (st == TRANSFER_RECEIVING) {
        /* 没有实时进度API，显示动画效果 */
        static uint8_t anim = 0;
        anim = (anim + 1) % 4;
        uint8_t pct = anim * 25;
        ui_draw_progress(ROW_PROGRESS, pct);
        ui_draw_progress(ROW_PROGRESS2, pct);
    }

    /* 槽位脏标记: 重绘 */
    if (g_slots_dirty) {
        g_slots_dirty = 0;
        ui_refresh_slots();
    }

    /* 命令反馈消息 */
    if (g_cmd_msg_valid) {
        char msg_line[MAX_COL + 1];
        snprintf(msg_line, sizeof(msg_line), "Message: %s", g_cmd_msg);
        ui_print_line(ROW_CMD_MSG, msg_line, g_cmd_color);
    }
}
