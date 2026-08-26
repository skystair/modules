/**
 * @file    xmodem.h
 * @brief   XMODEM-128/CRC 协议实现
 * @details 兼容小米IoT MCU OTA协议
 *          接收端实现，用于从PC接收固件文件
 */

#ifndef __XMODEM_H
#define __XMODEM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                              XMODEM协议常量                                 */
/* ========================================================================== */

/* 控制字符 */
#define XMODEM_SOH          0x01            /* 128字节数据包开始 */
#define XMODEM_STX          0x02            /* 1024字节数据包开始(未使用) */
#define XMODEM_EOT          0x04            /* 传输结束 */
#define XMODEM_ACK          0x06            /* 确认 */
#define XMODEM_NAK          0x15            /* 否定确认 */
#define XMODEM_CAN          0x18            /* 取消传输 */
#define XMODEM_EOF          0x1A            /* 填充字符 */
#define XMODEM_CRC_CHAR     'C'             /* 请求CRC模式 */

/* 包参数 */
#define XMODEM_DATA_SIZE    128             /* 数据字段大小 */
#define XMODEM_PACKET_SIZE  (1 + 1 + 1 + XMODEM_DATA_SIZE + 2)  /* SOH+PKT+~PKT+DATA+CRC = 133字节 */

/* 重试次数 */
#define XMODEM_MAX_RETRIES  10              /* 最大连续错误重试 */
#define XMODEM_MAX_ERRORS   99              /* 最大总错误次数 */

/* 超时时间(ms) */
#define XMODEM_TIMEOUT_MS   10000           /* 等待首包超时 */
#define XMODEM_CHAR_TIMEOUT 1000            /* 等待单字节超时(原3000ms,优化为1000ms) */

/* ========================================================================== */
/*                              返回码定义                                     */
/* ========================================================================== */

typedef enum {
    XMODEM_OK           =  0,               /* 传输成功 */
    XMODEM_ERR_TIMEOUT  = -1,               /* 超时 */
    XMODEM_ERR_CANCEL   = -2,               /* 对端取消 */
    XMODEM_ERR_RETRIES  = -3,               /* 重试次数过多 */
    XMODEM_ERR_PROTO    = -4,               /* 协议错误 */
    XMODEM_ERR_CRC      = -5,               /* CRC校验失败 */
    XMODEM_ERR_SEQ      = -6,               /* 序列号错误 */
    XMODEM_ERR_BUF      = -7,               /* 缓冲区不足 */
    XMODEM_ERR_IO       = -8,               /* IO错误 */
} xmodem_err_t;

/* ========================================================================== */
/*                              回调接口定义                                    */
/* ========================================================================== */

/**
 * @brief       发送单字节到串口
 * @param       ch: 要发送的字节
 */
typedef void (*xmodem_send_fn)(uint8_t ch);

/**
 * @brief       从串口接收单字节
 * @param       ch: 接收到的字节(输出)
 * @param       timeout_ms: 超时时间(毫秒)
 * @return      0=成功, -1=超时
 */
typedef int (*xmodem_recv_fn)(uint8_t *ch, uint32_t timeout_ms);

/**
 * @brief       接收到128字节数据块的处理回调
 * @param       seq: 包序列号
 * @param       data: 数据指针
 * @param       len: 数据长度
 * @return      0=成功, -1=失败
 */
typedef int (*xmodem_data_fn)(uint8_t seq, const uint8_t *data, uint32_t len);

/* ========================================================================== */
/*                              接收上下文                                     */
/* ========================================================================== */

typedef struct {
    /* 回调函数 */
    xmodem_send_fn  send;                   /* 发送字节 */
    xmodem_recv_fn  recv;                   /* 接收字节 */
    xmodem_data_fn  on_data;                /* 数据块回调 */

    /* 状态 */
    uint8_t  state;                         /* 状态机状态 */
    uint8_t  expected_seq;                  /* 期望的序列号 */
    uint16_t total_errors;                  /* 总错误计数 */
    uint16_t consecutive_errors;            /* 连续错误计数 */
    uint32_t total_bytes;                   /* 已接收总字节数 */

    /* 包缓冲区 */
    uint8_t  pkt_buf[XMODEM_PACKET_SIZE];   /* 包接收缓冲区 */
    uint16_t pkt_pos;                       /* 包内当前位置 */
} xmodem_recv_ctx_t;

/* ========================================================================== */
/*                              API接口声明                                    */
/* ========================================================================== */

/**
 * @brief       计算CRC16-Xmodem
 * @param       data: 数据指针
 * @param       len: 数据长度
 * @return      CRC16值
 */
uint16_t xmodem_crc16(const uint8_t *data, uint32_t len);

/**
 * @brief       初始化接收上下文
 * @param       ctx: 接收上下文指针
 * @param       send_fn: 发送函数
 * @param       recv_fn: 接收函数
 * @param       data_fn: 数据回调函数
 */
void xmodem_recv_init(xmodem_recv_ctx_t *ctx,
                      xmodem_send_fn send_fn,
                      xmodem_recv_fn recv_fn,
                      xmodem_data_fn data_fn);

/**
 * @brief       执行XMODEM接收
 * @param       ctx: 接收上下文指针
 * @return      接收的总字节数(成功), 负数=错误码
 * @note        阻塞调用，直到传输完成或失败
 */
int xmodem_recv(xmodem_recv_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* __XMODEM_H */
