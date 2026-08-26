/**
 * @file    transfer.c
 * @brief   USB传输控制模块实现
 * @details 处理PC命令，管理XMODEM接收，流式写入Flash
 */

#include "config.h"

/* ========================================================================== */
/*                              内部变量                                       */
/* ========================================================================== */

static transfer_state_t g_state = TRANSFER_IDLE;
static transfer_state_fn g_state_cb = NULL;
static int8_t g_selected_slot = -1;         /* 当前选中槽位 */

/* 命令缓冲区 */
#define CMD_BUF_SIZE        32
static uint8_t g_cmd_buf[CMD_BUF_SIZE];
static uint8_t g_cmd_pos = 0;

/* XMODEM上下文 */
static xmodem_recv_ctx_t g_xmodem_ctx;

/* 流式写入: 记录已写字节和CRC */
static uint32_t g_write_offset = 0;
static uint32_t g_write_crc = 0xFFFFFFFF;   /* CRC32运行值 */

/* ========================================================================== */
/*                              内部函数声明                                    */
/* ========================================================================== */

static void set_state(transfer_state_t state);
static void process_cmd(const uint8_t *cmd, uint32_t len);
static void handle_query(void);
static void handle_select(const uint8_t *cmd, uint32_t len);
static void handle_erase(void);
static void handle_transfer(const uint8_t *cmd, uint32_t len);
static void handle_delete(const uint8_t *cmd, uint32_t len);
static void handle_info(void);

/* XMODEM回调 */
static void xmodem_send(uint8_t ch);
static int xmodem_recv_byte(uint8_t *ch, uint32_t timeout_ms);
static int xmodem_on_data(uint8_t seq, const uint8_t *data, uint32_t len);

/* (USB数据读取已移至 modefunc_Lastfunc，此处不再单独读取) */

/* ========================================================================== */
/*                              状态管理                                       */
/* ========================================================================== */

static void set_state(transfer_state_t state)
{
    g_state = state;
    if (g_state_cb) {
        g_state_cb(state, -1);
    }
}

/* ========================================================================== */
/*                              命令处理                                       */
/* ========================================================================== */

static void process_cmd(const uint8_t *cmd, uint32_t len)
{
    if (len == 0) return;

    switch (cmd[0]) {
        case CMD_QUERY:
            handle_query();
            break;

        case CMD_SELECT:
            handle_select(cmd, len);
            break;

        case CMD_ERASE:
            if (g_selected_slot < 0) {
                transfer_send_resp(RESP_ERR, "No slot selected");
            } else {
                handle_erase();
            }
            break;

        case CMD_TRANSFER:
            if (g_selected_slot < 0) {
                transfer_send_resp(RESP_ERR, "No slot selected");
            } else {
                handle_transfer(cmd, len);
            }
            break;

        case CMD_DELETE:
            handle_delete(cmd, len);
            break;

        case CMD_INFO:
            handle_info();
            break;

        default:
            transfer_send_resp(RESP_ERR, "Unknown cmd");
            break;
    }
}

/**
 * @brief       处理查询命令 'Q'
 * @details     响应格式: "Q:slot:state:size:name\r\n"
 */
char resp[256];
static void handle_query(void)
{
    file_info_t info;
    int ret;
    
    char *p = resp;
    int remain = sizeof(resp);
    uint8_t i;
    int n;

    /* 拼接所有回复到一个缓冲区 */
    n = snprintf(p, remain, "K:QUERY\r\n");
    p += n; remain -= n;

    for (i = 0; i < SLOT_COUNT && remain > 0; i++) {
        ret = flash_store_get_info(i, &info);
        if (ret == FLASH_STORE_OK) {
            n = snprintf(p, remain, "SLOT %d: %u bytes [%s]\r\n",
                         i, info.size, info.name);  
        } else {
            n = snprintf(p, remain, "SLOT %d: [EMPTY]\r\n", i);
        }
        p += n; remain -= n;
    }

    /* 一次发送全部 */
    USB_CDC_Send((const uint8_t *)resp, (uint32_t)(p - resp));
}

/**
 * @brief       处理选择槽位命令 'S0'-'S4'
 */
static void handle_select(const uint8_t *cmd, uint32_t len)
{
    if (len < 2) {
        transfer_send_resp(RESP_ERR, "Missing slot");
        return;
    }

    uint8_t slot = cmd[1] - '0';
    if (slot >= SLOT_COUNT) {
        transfer_send_resp(RESP_ERR, "Invalid slot");
        return;
    }

    g_selected_slot = slot;
    set_state(TRANSFER_SELECTED);

    char resp[32];
    snprintf(resp, sizeof(resp), "Slot %d selected", slot);
    transfer_send_resp(RESP_OK, resp);
}

/**
 * @brief       处理擦除命令 'E'
 */
static void handle_erase(void)
{
    int ret = flash_store_delete(g_selected_slot);
    if (ret == FLASH_STORE_OK || ret == FLASH_STORE_ERR_EMPTY) {
        transfer_send_resp(RESP_OK, "Erased");
    } else {
        transfer_send_resp(RESP_ERR, "Erase failed");
    }
}

/**
 * @brief       处理开始传输命令 'T'
 * @details     进入XMODEM接收模式，流式写入Flash
 */
static void handle_transfer(const uint8_t *cmd, uint32_t len)
{
    if (g_state == TRANSFER_RECEIVING) {
        transfer_send_resp(RESP_ERR, "Busy");
        return;
    }

    /* 从命令中提取文件名: "T:filename" 或 "T" */
    const char *name = "firmware.bin";
    if (len > 2 && cmd[1] == ':') {
        name = (const char *)&cmd[2];
    }

    /* 准备流式写入: 擦除槽位 */
    int ret = flash_store_write_begin(g_selected_slot, name);
    if (ret != FLASH_STORE_OK) {
        transfer_send_resp(RESP_ERR, "Erase failed");
        return;
    }

    g_write_offset = 0;
    g_write_crc = 0xFFFFFFFF;
    set_state(TRANSFER_RECEIVING);
    transfer_send_resp(RESP_READY, "Send XMODEM");

    /* 初始化XMODEM接收 */
    xmodem_recv_init(&g_xmodem_ctx, xmodem_send, xmodem_recv_byte, xmodem_on_data);

    /* 进入Flash流式写入模式(避免每包Unlock/Lock) */
    flash_store_write_stream_begin();

    /* 开始接收(阻塞) */
    ret = xmodem_recv(&g_xmodem_ctx);

    /* 退出流式模式,锁定Flash */
    flash_store_write_stream_end();

    if (ret > 0) {
        /* 接收成功，完成写入 */
        uint32_t final_crc = g_write_crc ^ 0xFFFFFFFF;
        char resp[64];
        snprintf(resp, sizeof(resp), "Recv %u bytes", g_write_offset);
        transfer_send_resp(RESP_OK, resp);

        ret = flash_store_write_end(g_write_offset, final_crc);
        if (ret == FLASH_STORE_OK) {
            transfer_send_resp(RESP_OK, "Written");
            set_state(TRANSFER_DONE);
        } else {
            transfer_send_resp(RESP_ERR, "Write failed");
            set_state(TRANSFER_ERROR);
        }
    } else {
        /* 接收失败 */
        transfer_send_resp(RESP_ERR, "XMODEM failed");
        set_state(TRANSFER_ERROR);
    }

    g_selected_slot = -1;
}

/**
 * @brief       处理删除命令 'D0'-'D4'
 */
static void handle_delete(const uint8_t *cmd, uint32_t len)
{
    if (len < 2) {
        transfer_send_resp(RESP_ERR, "Missing slot");
        return;
    }

    uint8_t slot = cmd[1] - '0';
    if (slot >= SLOT_COUNT) {
        transfer_send_resp(RESP_ERR, "Invalid slot");
        return;
    }

    int ret = flash_store_delete(slot);
    if (ret == FLASH_STORE_OK) {
        transfer_send_resp(RESP_OK, "Deleted");
        set_state(TRANSFER_DELETED);
    } else {
        transfer_send_resp(RESP_ERR, "Delete failed");
    }
}

/**
 * @brief       处理信息命令 'I'
 */
static void handle_info(void)
{
    char resp[64];
    snprintf(resp, sizeof(resp), "Slots:%d Max:%dKB Files:%d",
             SLOT_COUNT, MAX_FILE_SIZE / 1024, flash_store_get_count());
    transfer_send_resp(RESP_OK, resp);
}

/* ========================================================================== */
/*                              XMODEM回调实现                                  */
/* ========================================================================== */

/* XMODEM 批量接收缓冲(优化: 减少USB CDC逐字节读取开销) */
static uint8_t g_xm_recv_buf[XMODEM_PACKET_SIZE];
static uint32_t g_xm_recv_pos = 0;
static uint32_t g_xm_recv_len = 0;

static void xmodem_send(uint8_t ch)
{
    USB_CDC_Send(&ch, 1);
}

static int xmodem_recv_byte(uint8_t *ch, uint32_t timeout_ms)
{
    uint32_t start = osKernelGetTickCount();

    for (;;) {
        /* 从批量缓冲中取出一个字节(零开销) */
        if (g_xm_recv_pos < g_xm_recv_len) {
            *ch = g_xm_recv_buf[g_xm_recv_pos++];
            return 0;
        }

        /* 缓冲为空,尝试批量填充 */
        g_xm_recv_pos = 0;
        g_xm_recv_len = 0;

        if (USB_CDC_DataAvailable()) {
            /* 一次读取所有可用数据(最多64字节=USB FS单次最大) */
            int n = USB_CDC_Receive(g_xm_recv_buf, sizeof(g_xm_recv_buf));
            if (n > 0) {
                g_xm_recv_len = (uint32_t)n;
                continue; /* 立即从缓冲取字节 */
            }
        }

        /* 超时检查 */
        if ((osKernelGetTickCount() - start) >= timeout_ms) {
            return -1;
        }

        /* 等待1ms: 让USB中断处理数据(比osDelay(0)更可靠) */
        osDelay(1);
    }
}

static int xmodem_on_data(uint8_t seq, const uint8_t *data, uint32_t len)
{
    /* 检查是否超出槽位空间 */
    if (g_write_offset + len > SLOT_SIZE) {
        return -1;
    }

    /* 流式写入Flash */
    int ret = flash_store_write_data(g_write_offset, data, len);
    if (ret != 0) {
        return -1;
    }

    /* 更新运行CRC32 */
    uint32_t i;
    for (i = 0; i < len; i++) {
        uint32_t j;
        g_write_crc ^= (uint32_t)data[i] << 24;
        for (j = 0; j < 8; j++) {
            if (g_write_crc & 0x80000000)
                g_write_crc = (g_write_crc << 1) ^ 0x04C11DB7;
            else
                g_write_crc <<= 1;
        }
    }

    g_write_offset += len;
    return 0;
}

/* ========================================================================== */
/*                              公共API实现                                    */
/* ========================================================================== */

void transfer_init(void)
{
    g_state = TRANSFER_IDLE;
    g_selected_slot = -1;
    g_cmd_pos = 0;
    g_write_offset = 0;
    g_write_crc = 0xFFFFFFFF;
}

void transfer_set_callback(transfer_state_fn cb)
{
    g_state_cb = cb;
}

void transfer_process(const uint8_t *data, uint32_t len)
{
    uint32_t i;

    /* 如果正在XMODEM接收，数据由xmodem_recv_byte处理 */
    if (g_state == TRANSFER_RECEIVING) {
        return;
    }

    /* 否则，解析ASCII命令 */
    for (i = 0; i < len; i++) {
        uint8_t ch = data[i];

        /* 忽略回车，换行作为命令结束 */
        if (ch == '\r') continue;

        if (ch == '\n') {
            if (g_cmd_pos > 0) {
                process_cmd(g_cmd_buf, g_cmd_pos);
                g_cmd_pos = 0;
            }
        } else {
            if (g_cmd_pos < CMD_BUF_SIZE - 1) {
                g_cmd_buf[g_cmd_pos++] = ch;
                g_cmd_buf[g_cmd_pos] = '\0';
            }
        }
    }
}

transfer_state_t transfer_get_state(void)
{
    return g_state;
}

int transfer_get_slot(void)
{
    return g_selected_slot;
}

void transfer_send_resp(uint8_t resp, const char *msg)
{
    char buf[64];
    if (msg) {
        snprintf(buf, sizeof(buf), "%c:%s\r\n", resp, msg);
    } else {
        snprintf(buf, sizeof(buf), "%c\r\n", resp);
    }
    USB_CDC_Send((const uint8_t *)buf, strlen(buf));
}
