#ifndef __USB_CDC_APP_H__
#define __USB_CDC_APP_H__

#include <stdint.h>

/* USB CDC ACM 初始化 */
void USB_CDC_Init(void);

/* USB CDC 发送数据（MCU → PC 串口助手） */
/* 返回实际写入发送缓冲的字节数，失败返回 -1 */
int USB_CDC_Send(const uint8_t *data, uint32_t len);

/* USB CDC 接收数据（PC 串口助手 → MCU） */
/* 返回实际读取的字节数，无数据返回 0 */
int USB_CDC_Receive(uint8_t *buf, uint32_t max_len);

/* 查询接收缓冲区是否有数据 */
uint8_t USB_CDC_DataAvailable(void);

/* 获取主机配置的波特率 */
uint32_t USB_CDC_GetBaudRate(void);

/* 获取主机配置的串口参数（完整） */
void USB_CDC_GetLineCoding(uint32_t *baud, uint8_t *stop, uint8_t *parity, uint8_t *databits);

#endif