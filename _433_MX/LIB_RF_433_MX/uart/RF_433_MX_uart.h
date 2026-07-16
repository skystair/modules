/**
 ******************************************************************************
 * @file    RF_433_MX_uart.h
 * @brief   433MHz OOK模块 UART协议层（参照STRUart + Uart_RXIT_funcL模式）
 * @details 本文件定义433模块的UART通信协议
 *          - STRUart 结构体管理收发状态
 *          - RX: Uart_RXIT_funcL 变长接收（sync→len→data→checksum）
 *          - TX: 阻塞发送（含前导码唤醒）
 *
 * 帧格式：  [0x55 × N preamble] [0x2D sync] [LEN] [DATA...] [CHECKSUM]
 * 校验和：  CHECKSUM = SYNC ^ LEN ^ DATA[0] ^ ... ^ DATA[N-1]
 *
 * 移植说明：
 *   1. 实现平台层4个函数：sendByte / waitTxEnd / txIntEnCtrl / rxEnCtrl
 *   2. 在平台层ISR中调用 Uart_RXIT_funcL() 接收字节
 *   3. 上层调用 RF_433_MX_uart_send() 发送，轮询 uart433.Rxcompflag 判断接收完成
 ******************************************************************************
 */

#ifndef __RF_433_MX_UART_h__
#define __RF_433_MX_UART_h__

/*==================== 帧协议配置 ====================*/
#define RF_433_MX_UART_MAX_DATA_LEN   32      /* 最大数据负载长度 */
#define RF_433_MX_UART_PREAMBLE_BYTE  0x55    /* 前导码字节 */
#define RF_433_MX_UART_PREAMBLE_LEN   10      /* 前导码字节数 */
#define RF_433_MX_UART_SYNC_WORD      0x2D    /* 同步字 */
#define RF_433_MX_UART_RXL_MAX        100     /* 变长接收最大长度限制 */

/*==================== 平台层接口（由具体平台实现） ====================*/
void RF_433_MX_uart_sendByte(unsigned char byte);   /* 发送1字节（阻塞） */
void RF_433_MX_uart_waitTxEnd(void);                /* 等待发送完成 */
void RF_433_MX_uart_txIntEnCtrl(unsigned char en);  /* TX中断使能控制 */
void RF_433_MX_uart_rxEnCtrl(unsigned char en);     /* RX使能控制 */
void RF_433_MX_uart_enableWakeup(void);             /* 使能EXTI唤醒中断 */
void RF_433_MX_uart_platformInit(void);              /* 平台层初始化（由init内部调用） */

/*==================== 协议层接口 ====================*/
void RF_433_MX_uart_init(void);                     /* 统一初始化入口 */
void RF_433_MX_uart_func(void);
void RF_433_MX_uart_TXtrig(void);
/*==================== Watch窗口观察变量（extern） ====================*/
extern unsigned char rxBuffer[32];    /* 最近一帧数据副本 */
extern unsigned char rxLen;           /* 最近一帧数据长度 */
extern unsigned int  rxPktCount;      /* 累计接收帧数 */

#endif
