/**
 * RF_433_MX_test_rx.c
 * 接收端测试用例 — 封装版
 * 已废弃：UART方案使用 RF_433_MX_uart.c 替代
 * 如需编译此文件，请移除下方 #if 0 或从工程中移除
 */
#if 1  /* UART方案启用时，屏蔽旧测试代码避免符号冲突 */

#include "config.h"
#include "RF_433_MX_test_rx.h"

/*==================== 接收缓冲区（外部可观察） ====================*/
unsigned char rxBuffer[RF_433_MX_RX_MAX_DATA_LEN];
unsigned char rxLen = 0;
unsigned int  rxPktCount = 0;

/*==================== 历史包记录 ====================*/
#define RX_PKT_LOG_SIZE  10
typedef struct {
    unsigned char data[RF_433_MX_RX_MAX_DATA_LEN];
    unsigned char len;
    unsigned char valid;
} RxPktRecord;

static RxPktRecord rxPktLog[RX_PKT_LOG_SIZE];
static unsigned char rxPktLogIndex = 0;

static void logPacket(unsigned char *data, unsigned char len) {
    unsigned char i;
    RxPktRecord *rec = &rxPktLog[rxPktLogIndex];

    for (i = 0; i < len && i < RF_433_MX_RX_MAX_DATA_LEN; i++) {
        rec->data[i] = data[i];
    }
    rec->len   = len;
    rec->valid = 1;

    rxPktLogIndex++;
    if (rxPktLogIndex >= RX_PKT_LOG_SIZE) {
        rxPktLogIndex = 0;
    }
}

/*==================== 接口函数 ====================*/

void RF_433_MX_test_rx_init(void) {
    unsigned char i;
    rxLen      = 0;
    rxPktCount = 0;
    rxPktLogIndex = 0;
    for (i = 0; i < RX_PKT_LOG_SIZE; i++) {
        rxPktLog[i].valid = 0;
        rxPktLog[i].len   = 0;
    }
}

void RF_433_MX_test_rx_poll(void) {
    unsigned char i;
    unsigned char *pData;

    if (!RF_433_MX_RX_isDone()) return;

    pData = RF_433_MX_RX_getData(&rxLen);

    if (pData != 0 && rxLen > 0) {
        for (i = 0; i < rxLen; i++) {
            rxBuffer[i] = pData[i];
        }
        logPacket(rxBuffer, rxLen);
//        memcpy(rxBuffer,rxCtrl,);
        rxPktCount++;
        /* 断点位置：rxBuffer[0..rxLen-1] */
    }
    LEDxCtrl(LED_CH_W, LED_STATE_FLASH, 1000);
    RF_433_MX_RX_clearDone();
}

#endif  /* UART方案：屏蔽旧测试代码 */
