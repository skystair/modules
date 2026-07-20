/**
 ******************************************************************************
 * @file    dlibx_uart.h
 * @brief   通用 UART 通信模块（基于 STRUart 框架）
 * @details
 *   基于 dlibx.h 的 STRUart 结构体，提供开箱即用的帧收发功能。
 *   参照 RF_433_MX_uart 的设计模式，但去除 433MHz 专用逻辑，适用于任意 UART 通信。
 *
 *   帧格式：[H1] [H2] [H3] [LEN] [DATA...] [CHECKSUM]
 *     TX帧头顺序: TX_H1, TX_H2, TX_H3
 *     RX帧头顺序: RX_H1, RX_H2, RX_H3（与TX不同，用于区分对端）
 *   - LEN: DATA 部分长度
 *   - CHECKSUM: H1 + H2 + H3 + LEN + DATA[0] + ... + DATA[N-1]（求和取低8位）
 *
 *   移植步骤：
 *     1. 实现平台层函数：dlibx_uart_sendByte / waitTxEnd / txIntEnCtrl / rxEnCtrl
 *     2. 实现 dlibx_uart_platformInit() 完成硬件初始化（GPIO、USART、NVIC）
 *     3. 在 USART RXNE 中断中调用：
 *          uart_str.Rxtmp = 读取数据寄存器;
 *          Uart_RXIT_funcL(&uart_str, uart_str.RxDlenPos);
 *     4. 在 USART TXE 中断中调用：
 *          Uart_TXIT_func(&uart_str);
 *
 *   最大负载长度由 DLIBX_UART_MAX_DATA_LEN 控制（默认 32 字节）
 ******************************************************************************
 */

#ifndef __dlibx_uart_h__
#define __dlibx_uart_h__

#include "dlibx.h"

/*==================== 帧协议配置（TX/RX 帧头顺序不同，用于区分收发方） ====================*/
#define DLIBX_UART_MAX_DATA_LEN     32      /* 最大数据负载长度 */
#define DLIBX_UART_RXL_MAX          100     /* 变长接收最大长度限制 */

/*--- TX 帧头（本机发送时使用） ---*/
#define DLIBX_UART_TX_H1            0xAA
#define DLIBX_UART_TX_H2            0x55
#define DLIBX_UART_TX_H3            0x66

/*--- RX 帧头（本机接收时匹配，顺序与TX不同以区分对端） ---*/
#define DLIBX_UART_RX_H1            0xAA
#define DLIBX_UART_RX_H2            0x66
#define DLIBX_UART_RX_H3            0x55

/*==================== STRUart 初始化参数（默认宏） ====================*/
#define DLIBX_UART_REN_AFTER_TX     1       /* 发送完成后自动使能 RX */
#define DLIBX_UART_RX_LEN_INIT      10      /* RX初始最小帧长，接收 LEN 后会动态更新 */
#define DLIBX_UART_TX_LEN_INIT      10      /* TX初始最小帧长 */
#define DLIBX_UART_TX_TICK_TRIG     100     /* 发送超时计数上限（单位：周期数） */

/* per-default-instance (exp1) macros — keep values consistent but provide exp1-prefixed names */
#define UART_EXP1_REN_AFTER_TX      DLIBX_UART_REN_AFTER_TX
#define UART_EXP1_RX_LEN_INIT       DLIBX_UART_RX_LEN_INIT
#define UART_EXP1_TX_LEN_INIT       DLIBX_UART_TX_LEN_INIT
#define UART_EXP1_TX_TICK_TRIG      DLIBX_UART_TX_TICK_TRIG

/*==================== 平台层接口（由具体平台实现） ====================*/
void dlibx_uart_sendByte(unsigned char byte);       /* 发送1字节 */
void dlibx_uart_waitTxEnd(void);                    /* 等待发送完成 */
void dlibx_uart_txIntEnCtrl(unsigned char en);      /* TX中断使能控制 */
void dlibx_uart_rxEnCtrl(unsigned char en);         /* RX使能控制 */
void dlibx_uart_platformInit(void);                  /* 平台层初始化 */

/*==================== 协议层接口 ====================*/
void dlibx_uart_init(void);                         /* 使用默认配置初始化（仅做平台初始化） */
void dlibx_uart_initEx(STRUart* uart, const dlibx_uart_cfg_t* cfg); /* 使用指定配置初始化并应用到实例 */
void dlibx_uart_TXfunc(STRUart* uart);             /* 触发发送流程（对指定实例） */
void dlibx_uart_RXfunc(STRUart* uart);             /* 触发接收校验（对指定实例） */

/* 周期处理函数（在主循环或任务中调用） */
void dlibx_uart_func(void);

/*==================== 接收状态查询 ====================*/
extern STRUart uart_exp1;                            /* 默认 UART 实例（保留兼容的单实例名） */

/* 旧的全局 rxData / rxLen 不再导出；请通过实例访问：uart.rxData / uart.rxDataLen */

#endif
