#include "rl_usb.h"
#include "usb_cdc_app.h"
#include <string.h>

/* CDC LINE_CODING 缓存（由主机 SET_LINE_CODING 更新） */
static CDC_LINE_CODING cdc_line_coding = {
    115200U,   /* 默认波特率 */
    0U,        /* 1 停止位 */
    0U,        /* 无校验 */
    8U         /* 8 数据位 */
};

/* DTR/RTS 控制线状态 */
static volatile uint8_t cdc_control_line_state = 0U;

/*==========================================================================
 * MDK-Middleware CDC ACM 回调函数
 * 函数名由 rl_usb.h 规范定义，链接时自动绑定
 *==========================================================================*/

/* CDC 初始化回调 */
void USBD_CDC0_ACM_Initialize(void) {
    cdc_control_line_state = 0U;
}

/* CDC 去初始化回调 */
void USBD_CDC0_ACM_Uninitialize(void) {
    cdc_control_line_state = 0U;
}

/* CDC 复位回调 */
void USBD_CDC0_ACM_Reset(void) {
    cdc_control_line_state = 0U;
}

/* 主机发送 SET_LINE_CODING：更新串口参数 */
bool USBD_CDC0_ACM_SetLineCoding(const CDC_LINE_CODING *line_coding) {
    if (line_coding == NULL) {
        return false;
    }
    memcpy(&cdc_line_coding, line_coding, sizeof(CDC_LINE_CODING));
    return true;
}

/* 主机请求 GET_LINE_CODING：返回当前串口参数 */
bool USBD_CDC0_ACM_GetLineCoding(CDC_LINE_CODING *line_coding) {
    if (line_coding == NULL) {
        return false;
    }
    memcpy(line_coding, &cdc_line_coding, sizeof(CDC_LINE_CODING));
    return true;
}

/* 主机发送 SET_CONTROL_LINE_STATE：DTR/RTS 变化 */
bool USBD_CDC0_ACM_SetControlLineState(uint16_t state) {
    cdc_control_line_state = (uint8_t)(state & 0x03U);
    return true;
}

/* 数据接收完成回调（主机发来数据） */
void USBD_CDC0_ACM_DataReceived(uint32_t len) {
    (void)len;
    /* 由 USBD_CDC_ACM_ReadData 在 task 层读取 */
}

/* 数据发送完成回调 */
void USBD_CDC0_ACM_DataSent(void) {
}

/*==========================================================================
 * 应用层 API
 *==========================================================================*/

void USB_CDC_Init(void) {
    USBD_Initialize(0U);
    USBD_Connect(0U);
}

int USB_CDC_Send(const uint8_t *data, uint32_t len) {
    if (data == NULL || len == 0U) {
        return -1;
    }
    int32_t ret = USBD_CDC_ACM_WriteData(0U, data, len);
    return (int)ret;
}

int USB_CDC_Receive(uint8_t *buf, uint32_t max_len) {
    if (buf == NULL || max_len == 0U) {
        return 0;
    }
    int32_t ret = USBD_CDC_ACM_ReadData(0U, buf, max_len);
    if (ret < 0) {
        return 0;
    }
    return (int)ret;
}

uint8_t USB_CDC_DataAvailable(void) {
    return (uint8_t)USBD_CDC_ACM_DataAvailable(0U);
}

uint32_t USB_CDC_GetBaudRate(void) {
    return cdc_line_coding.dwDTERate;
}

void USB_CDC_GetLineCoding(uint32_t *baud, uint8_t *stop, uint8_t *parity, uint8_t *databits) {
    if (baud)     *baud     = cdc_line_coding.dwDTERate;
    if (stop)     *stop     = cdc_line_coding.bCharFormat;
    if (parity)   *parity   = cdc_line_coding.bParityType;
    if (databits) *databits = cdc_line_coding.bDataBits;
}