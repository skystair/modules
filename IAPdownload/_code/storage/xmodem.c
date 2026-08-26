/**
 * @file    xmodem.c
 * @brief   XMODEM-128/CRC 协议实现
 * @details 兼容小米IoT MCU OTA协议
 *          包格式: SOH | PKT# | ~PKT# | 128字节数据 | CRC_H | CRC_L
 */

#include "config.h"

/* ========================================================================== */
/*                              状态机定义                                     */
/* ========================================================================== */

#define STATE_INIT          0               /* 初始化，发送'C' */
#define STATE_WAIT_SOH      1               /* 等待SOH或EOT */
#define STATE_RECV_PKT      2               /* 接收包序列号 */
#define STATE_RECV_NPKT     3               /* 接收包序列号反码 */
#define STATE_RECV_DATA     4               /* 接收128字节数据 */
#define STATE_RECV_CRC_H    5               /* 接收CRC高字节 */
#define STATE_RECV_CRC_L    6               /* 接收CRC低字节 */
#define STATE_VERIFY        7               /* 验证数据包 */
#define STATE_DONE          8               /* 传输完成 */

/* ========================================================================== */
/*                              CRC16-Xmodem计算                               */
/* ========================================================================== */

uint16_t xmodem_crc16(const uint8_t *data, uint32_t len)
{
    uint16_t crc = 0x0000;                  /* XMODEM初始值0 */
    uint32_t i, j;

    for (i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;     /* 字节移入高8位 */
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;  /* CRC-CCITT多项式 */
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

/* ========================================================================== */
/*                              内部辅助函数                                    */
/* ========================================================================== */

/**
 * @brief       发送CAN(取消传输)
 */
static void send_can(xmodem_recv_ctx_t *ctx)
{
    ctx->send(XMODEM_CAN);
    ctx->send(XMODEM_CAN);
    ctx->send(XMODEM_CAN);
}

/**
 * @brief       重置包状态
 */
static void reset_packet(xmodem_recv_ctx_t *ctx)
{
    ctx->pkt_pos = 0;
    ctx->state = STATE_WAIT_SOH;
}

/**
 * @brief       接收一个字节(带超时)
 * @return      0=成功, -1=超时
 */
static int recv_byte(xmodem_recv_ctx_t *ctx, uint8_t *ch)
{
    return ctx->recv(ch, XMODEM_CHAR_TIMEOUT);
}

/* ========================================================================== */
/*                              API实现                                        */
/* ========================================================================== */

void xmodem_recv_init(xmodem_recv_ctx_t *ctx,
                      xmodem_send_fn send_fn,
                      xmodem_recv_fn recv_fn,
                      xmodem_data_fn data_fn)
{
    memset(ctx, 0, sizeof(xmodem_recv_ctx_t));
    ctx->send = send_fn;
    ctx->recv = recv_fn;
    ctx->on_data = data_fn;
    ctx->expected_seq = 1;                  /* XMODEM从1开始 */
    ctx->state = STATE_INIT;
}

int xmodem_recv(xmodem_recv_ctx_t *ctx)
{
    uint8_t ch;
    uint16_t crc_calc, crc_recv;
    int ret;
    uint32_t retry_timer;

    while (ctx->state != STATE_DONE) {

        /* 检查总错误次数 */
        if (ctx->total_errors >= XMODEM_MAX_ERRORS) {
            send_can(ctx);
            return XMODEM_ERR_RETRIES;
        }

        switch (ctx->state) {

        /* ==================== 初始化阶段 ==================== */
        case STATE_INIT:
            /* 发送'C'请求CRC模式 */
            ctx->send(XMODEM_CRC_CHAR);
            ctx->pkt_pos = 0;
            ctx->state = STATE_WAIT_SOH;
            break;

        /* ==================== 等待SOH/EOT ==================== */
        case STATE_WAIT_SOH:
            if (recv_byte(ctx, &ch) != 0) {
                /* 超时，重发'C' */
                ctx->consecutive_errors++;
                ctx->total_errors++;
                if (ctx->consecutive_errors >= XMODEM_MAX_RETRIES) {
                    send_can(ctx);
                    return XMODEM_ERR_TIMEOUT;
                }
                ctx->send(XMODEM_CRC_CHAR);
                break;
            }

            if (ch == XMODEM_SOH) {
                /* 收到SOH，开始接收包 */
                ctx->pkt_buf[0] = ch;
                ctx->pkt_pos = 1;
                ctx->state = STATE_RECV_PKT;
            } else if (ch == XMODEM_EOT) {
                /* 传输结束 */
                ctx->send(XMODEM_ACK);
                ctx->state = STATE_DONE;
            } else if (ch == XMODEM_CAN) {
                /* 对端取消 */
                return XMODEM_ERR_CANCEL;
            }
            /* 其他字节忽略，继续等待 */
            break;

        /* ==================== 接收包序列号 ==================== */
        case STATE_RECV_PKT:
            if (recv_byte(ctx, &ch) != 0) {
                reset_packet(ctx);
                ctx->consecutive_errors++;
                ctx->total_errors++;
                break;
            }
            ctx->pkt_buf[1] = ch;
            ctx->state = STATE_RECV_NPKT;
            break;

        /* ==================== 接收包序列号反码 ==================== */
        case STATE_RECV_NPKT:
            if (recv_byte(ctx, &ch) != 0) {
                reset_packet(ctx);
                ctx->consecutive_errors++;
                ctx->total_errors++;
                break;
            }
            ctx->pkt_buf[2] = ch;

            /* 验证反码 */
            if ((uint8_t)(ctx->pkt_buf[1] + ch) != 0xFF) {
                /* 序列号反码错误 */
                ctx->send(XMODEM_NAK);
                ctx->consecutive_errors++;
                ctx->total_errors++;
                reset_packet(ctx);
                break;
            }
            ctx->pkt_pos = 3;
            ctx->state = STATE_RECV_DATA;
            break;

        /* ==================== 接收128字节数据 ==================== */
        case STATE_RECV_DATA:
            if (recv_byte(ctx, &ch) != 0) {
                reset_packet(ctx);
                ctx->consecutive_errors++;
                ctx->total_errors++;
                break;
            }
            ctx->pkt_buf[ctx->pkt_pos++] = ch;

            if (ctx->pkt_pos >= 3 + XMODEM_DATA_SIZE) {
                ctx->state = STATE_RECV_CRC_H;
            }
            break;

        /* ==================== 接收CRC高字节 ==================== */
        case STATE_RECV_CRC_H:
            if (recv_byte(ctx, &ch) != 0) {
                reset_packet(ctx);
                ctx->consecutive_errors++;
                ctx->total_errors++;
                break;
            }
            ctx->pkt_buf[3 + XMODEM_DATA_SIZE] = ch;
            ctx->state = STATE_RECV_CRC_L;
            break;

        /* ==================== 接收CRC低字节 ==================== */
        case STATE_RECV_CRC_L:
            if (recv_byte(ctx, &ch) != 0) {
                reset_packet(ctx);
                ctx->consecutive_errors++;
                ctx->total_errors++;
                break;
            }
            ctx->pkt_buf[3 + XMODEM_DATA_SIZE + 1] = ch;
            ctx->state = STATE_VERIFY;
            break;

        /* ==================== 验证数据包 ==================== */
        case STATE_VERIFY:
        {
            uint8_t pkt_seq = ctx->pkt_buf[1];
            uint8_t *data = &ctx->pkt_buf[3];

            /* 计算CRC */
            crc_calc = xmodem_crc16(data, XMODEM_DATA_SIZE);
            crc_recv = ((uint16_t)ctx->pkt_buf[3 + XMODEM_DATA_SIZE] << 8)
                     | ctx->pkt_buf[3 + XMODEM_DATA_SIZE + 1];

            if (crc_calc != crc_recv) {
                /* CRC错误 */
                ctx->send(XMODEM_NAK);
                ctx->consecutive_errors++;
                ctx->total_errors++;
                reset_packet(ctx);
                break;
            }

            /* 检查序列号 */
            if (pkt_seq == (uint8_t)(ctx->expected_seq - 1)) {
                /* 重复包，直接ACK */
                ctx->send(XMODEM_ACK);
                ctx->consecutive_errors = 0;
                reset_packet(ctx);
                break;
            }

            if (pkt_seq != ctx->expected_seq) {
                /* 序列号不匹配 */
                send_can(ctx);
                return XMODEM_ERR_SEQ;
            }

            /* 数据有效，调用回调 */
            ret = ctx->on_data(pkt_seq, data, XMODEM_DATA_SIZE);
            if (ret != 0) {
                send_can(ctx);
                return XMODEM_ERR_IO;
            }

            /* 更新状态 */
            ctx->expected_seq++;
            ctx->total_bytes += XMODEM_DATA_SIZE;
            ctx->consecutive_errors = 0;
            ctx->send(XMODEM_ACK);
            reset_packet(ctx);
            break;
        }

        default:
            ctx->state = STATE_INIT;
            break;
        }
    }

    return (int)ctx->total_bytes;
}
