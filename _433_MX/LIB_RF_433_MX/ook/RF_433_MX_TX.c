#include "RF_433_MX_TX.h"

/* 全局控制结构体 */
RF_433_MX_TX_Ctrl txCtrl;

/* 前导码字节：0xAA (10101010) */
#define PREAMBLE_BYTE 0xAA

/* 校验和计算：从同步字开始，包括数据长度和数据字节 */
static unsigned char RF_433_MX_TX_calcChecksum(unsigned char *data, unsigned char len) {
    unsigned char checksum = RF_433_MX_TX_SYNC_WORD ^ len;
    unsigned char i;
    for (i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

/* 初始化 */
void RF_433_MX_TX_init(void) {
    txCtrl.state = RF_433_MX_TX_IDLE;
    txCtrl.data = 0;
    txCtrl.dataLen = 0;
    txCtrl.byteIndex = 0;
    txCtrl.bitIndex = 0;
    txCtrl.preambleCount = 0;
    txCtrl.checksum = 0;
    /* 硬件初始化：设置发射引脚为输出，并默认低电平 */
    /* 平台层需实现 RF_433_MX_TX_SetPin() */
}

/* 发射数据函数 */
unsigned char RF_433_MX_TX_send(unsigned char *data, unsigned char len) {
    if (txCtrl.state != RF_433_MX_TX_IDLE) {
        return 0; // 忙
    }
    if (len > RF_433_MX_TX_MAX_DATA_LEN) {
        return 0; // 数据过长
    }
    txCtrl.data = data;
    txCtrl.dataLen = len;
    txCtrl.checksum = RF_433_MX_TX_calcChecksum(data, len);

    txCtrl.state = RF_433_MX_TX_PREAMBLE;
    txCtrl.preambleCount = 0;
    txCtrl.byteIndex = 0;
    txCtrl.bitIndex = 7; // 从MSB开始
    return 1; // 成功开始发送
}

/* 1ms定时器tick函数（由定时器中断或主循环每1ms调用一次） */
void RF_433_MX_TX_tick1ms(void) {
    if (txCtrl.state == RF_433_MX_TX_IDLE) {
        return;
    }

    /* 每次 tick 直接处理 1 bit（1bit = 1ms = 1 tick） */

    switch (txCtrl.state) {
        case RF_433_MX_TX_PREAMBLE:
            RF_433_MX_TX_SetPin((PREAMBLE_BYTE >> txCtrl.bitIndex) & 0x01);
            if (txCtrl.bitIndex == 0) {
                txCtrl.bitIndex = 7;
                txCtrl.preambleCount++;
                if (txCtrl.preambleCount >= RF_433_MX_TX_PREAMBLE_LEN) {
                    txCtrl.state = RF_433_MX_TX_SYNC;
                    txCtrl.byteIndex = 0;
                    txCtrl.bitIndex = 7;
                }
            } else {
                txCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_TX_SYNC:
            RF_433_MX_TX_SetPin((RF_433_MX_TX_SYNC_WORD >> txCtrl.bitIndex) & 0x01);
            if (txCtrl.bitIndex == 0) {
                txCtrl.state = RF_433_MX_TX_DATA_LEN;
                txCtrl.bitIndex = 7;
            } else {
                txCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_TX_DATA_LEN:
            RF_433_MX_TX_SetPin((txCtrl.dataLen >> txCtrl.bitIndex) & 0x01);
            if (txCtrl.bitIndex == 0) {
                if (txCtrl.dataLen == 0) {
                    txCtrl.state = RF_433_MX_TX_CHECKSUM;
                } else {
                    txCtrl.state = RF_433_MX_TX_DATA;
                    txCtrl.byteIndex = 0;
                }
                txCtrl.bitIndex = 7;
            } else {
                txCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_TX_DATA:
            RF_433_MX_TX_SetPin((txCtrl.data[txCtrl.byteIndex] >> txCtrl.bitIndex) & 0x01);
            if (txCtrl.bitIndex == 0) {
                txCtrl.byteIndex++;
                if (txCtrl.byteIndex >= txCtrl.dataLen) {
                    txCtrl.state = RF_433_MX_TX_CHECKSUM;
                }
                txCtrl.bitIndex = 7;
            } else {
                txCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_TX_CHECKSUM:
            RF_433_MX_TX_SetPin((txCtrl.checksum >> txCtrl.bitIndex) & 0x01);
            if (txCtrl.bitIndex == 0) {
                txCtrl.state = RF_433_MX_TX_CHECKSUM2;
                txCtrl.bitIndex = 7;
            } else {
                txCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_TX_CHECKSUM2:
            RF_433_MX_TX_SetPin((txCtrl.checksum >> txCtrl.bitIndex) & 0x01);
            if (txCtrl.bitIndex == 0) {
                txCtrl.state = RF_433_MX_TX_DONE;
                RF_433_MX_TX_SetPin(0);
            } else {
                txCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_TX_DONE:
            txCtrl.state = RF_433_MX_TX_IDLE;
            break;

        default:
            txCtrl.state = RF_433_MX_TX_IDLE;
            break;
    }
}

/* 查询发射器是否空闲 */
unsigned char RF_433_MX_TX_isIdle(void) {
    return (txCtrl.state == RF_433_MX_TX_IDLE) ? 1 : 0;
}

/* 主循环函数 */
void RF_433_MX_TX_func(void) {
    /* 可以在这里检查发送状态，执行回调等 */
}

/* 硬件抽象层函数：在平台适配层中实现 */
/* extern 声明见 RF_433_MX_TX.h */
