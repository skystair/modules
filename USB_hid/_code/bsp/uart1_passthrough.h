#ifndef __UART1_PASSTHROUGH_H__
#define __UART1_PASSTHROUGH_H__

#include <stdint.h>

/* 初始化 USART1（PA9=TX, PA10=RX） */
void UART1_Passthrough_Init(void);

/* 动态配置 USART1 波特率、数据位、停止位、校验 */
void UART1_Passthrough_SetConfig(uint32_t baud, uint8_t databits, uint8_t stopbits, uint8_t parity);

/* UART1 发送数据（阻塞，等待 TXE） */
void UART1_Passthrough_Send(const uint8_t *data, uint32_t len);

/* 查询 UART1 接收缓冲区已接收字节数 */
uint32_t UART1_Passthrough_RxCount(void);

/* 从 UART1 接收缓冲区读取数据，返回实际读取字节数 */
uint32_t UART1_Passthrough_Receive(uint8_t *buf, uint32_t max_len);

#endif