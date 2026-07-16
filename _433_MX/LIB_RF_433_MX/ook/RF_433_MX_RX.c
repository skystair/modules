#include "RF_433_MX_RX.h"

/*
 * MX-05V 接收器驱动 — ISR 驱动架构
 *
 * 时序原理：
 *   前导码 0xAA = 10101010，每个上升沿间隔 1ms
 *   → EXTI 上升沿中断自动对齐 bit 边界
 *   → TIM 在 500μs 后（bit 中心）采样，得到最稳定的值
 *
 *   ┌──┐  ┌──┐  ┌──┐  ┌──┐     ← 前导码波形
 *   │  │  │  │  │  │  │  │
 * ──┘  └──┘  └──┘  └──┘  └──
 *   ↑     ↑     ↑     ↑         ← EXTI 上升沿（同步锚点）
 *    ↓     ↓     ↓     ↓        ← 500μs 后 TIM 中断采样（bit 中心）
 */

/*==================== 全局控制 ====================*/
RF_433_MX_RX_Ctrl rxCtrl;

/*==================== 初始化 ====================*/
void RF_433_MX_RX_init(void) {
    rxCtrl.state        = RF_433_MX_RX_IDLE;
    rxCtrl.dataLen      = 0;
    rxCtrl.byteIndex    = 0;
    rxCtrl.bitIndex     = 7;
    rxCtrl.preambleCount = 0;
    rxCtrl.receivedByte = 0;
    rxCtrl.checksum     = 0;
    rxCtrl.calcChecksum = 0;
    rxCtrl.timeoutCnt   = 0;
    rxCtrl.synced       = 0;
}

/*==================== 外部中断回调 ====================*/
void RF_433_MX_RX_onRisingEdge(void) {
    if (rxCtrl.state == RF_433_MX_RX_IDLE) {
        rxCtrl.state         = RF_433_MX_RX_PREAMBLE;
        rxCtrl.preambleCount = 0;
        rxCtrl.bitIndex      = 7;
        rxCtrl.receivedByte  = 0;
        rxCtrl.synced        = 1;
    }
    rxCtrl.timeoutCnt = 0;
}

/*==================== 超时检测 ====================*/
void RF_433_MX_RX_checkTimeout(void) {
    if (rxCtrl.state == RF_433_MX_RX_IDLE) {
        return;
    }
    rxCtrl.timeoutCnt++;
    if (rxCtrl.timeoutCnt >= RF_433_MX_RX_TIMEOUT_MS) {
        rxCtrl.state        = RF_433_MX_RX_IDLE;
        rxCtrl.synced       = 0;
        rxCtrl.timeoutCnt   = 0;
    }
}

/*==================== bit 处理（状态机核心） ====================*/
void RF_433_MX_RX_processBit(unsigned char bitValue) {
    switch (rxCtrl.state) {

        case RF_433_MX_RX_PREAMBLE: {
            unsigned char expected;
            expected = rxCtrl.bitIndex & 0x01;
            if (bitValue != expected) {
                rxCtrl.state = RF_433_MX_RX_IDLE;
                rxCtrl.synced = 0;
                break;
            }

            if (bitValue) {
                rxCtrl.receivedByte |= (1u << rxCtrl.bitIndex);
            }

            if (rxCtrl.bitIndex == 0) {
                if (rxCtrl.receivedByte == 0xAA) {
                    rxCtrl.preambleCount++;
                    if (rxCtrl.preambleCount >= RF_433_MX_RX_PREAMBLE_LEN) {
                        rxCtrl.state        = RF_433_MX_RX_SYNC;
                        rxCtrl.calcChecksum = 0;
                    }
                } else {
                    rxCtrl.state = RF_433_MX_RX_IDLE;
                    rxCtrl.synced = 0;
                    break;
                }
                rxCtrl.receivedByte = 0;
                rxCtrl.bitIndex = 7;
            } else {
                rxCtrl.bitIndex--;
            }
            break;
        }

        case RF_433_MX_RX_SYNC:
            if (bitValue) {
                rxCtrl.receivedByte |= (1u << rxCtrl.bitIndex);
            }
            if (rxCtrl.bitIndex == 0) {
                if (rxCtrl.receivedByte == RF_433_MX_RX_SYNC_WORD) {
                    rxCtrl.calcChecksum ^= rxCtrl.receivedByte;
                    rxCtrl.state        = RF_433_MX_RX_DATA_LEN;
                } else {
                    rxCtrl.state  = RF_433_MX_RX_IDLE;
                    rxCtrl.synced = 0;
                }
                rxCtrl.receivedByte = 0;
                rxCtrl.bitIndex = 7;
            } else {
                rxCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_RX_DATA_LEN:
            if (bitValue) {
                rxCtrl.receivedByte |= (1u << rxCtrl.bitIndex);
            }
            if (rxCtrl.bitIndex == 0) {
                rxCtrl.dataLen = rxCtrl.receivedByte;
                rxCtrl.calcChecksum ^= rxCtrl.dataLen;

                if (rxCtrl.dataLen > RF_433_MX_RX_MAX_DATA_LEN) {
                    rxCtrl.state  = RF_433_MX_RX_IDLE;
                    rxCtrl.synced = 0;
                } else if (rxCtrl.dataLen == 0) {
                    rxCtrl.state = RF_433_MX_RX_CHECKSUM;
                } else {
                    rxCtrl.state     = RF_433_MX_RX_DATA;
                    rxCtrl.byteIndex = 0;
                }
                rxCtrl.receivedByte = 0;
                rxCtrl.bitIndex = 7;
            } else {
                rxCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_RX_DATA:
            if (bitValue) {
                rxCtrl.receivedByte |= (1u << rxCtrl.bitIndex);
            }
            if (rxCtrl.bitIndex == 0) {
                rxCtrl.buffer[rxCtrl.byteIndex] = rxCtrl.receivedByte;
                rxCtrl.calcChecksum ^= rxCtrl.receivedByte;
                rxCtrl.byteIndex++;

                if (rxCtrl.byteIndex >= rxCtrl.dataLen) {
                    rxCtrl.state = RF_433_MX_RX_CHECKSUM;
                }
                rxCtrl.receivedByte = 0;
                rxCtrl.bitIndex = 7;
            } else {
                rxCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_RX_CHECKSUM:
            if (bitValue) {
                rxCtrl.receivedByte |= (1u << rxCtrl.bitIndex);
            }
            if (rxCtrl.bitIndex == 0) {
                rxCtrl.checksum = rxCtrl.receivedByte;

                if (rxCtrl.checksum == rxCtrl.calcChecksum) {
                    rxCtrl.state = RF_433_MX_RX_CHECKSUM_SKIP;
                } else {
                    rxCtrl.state  = RF_433_MX_RX_IDLE;
                    rxCtrl.synced = 0;
                }
                rxCtrl.receivedByte = 0;
                rxCtrl.bitIndex = 7;
            } else {
                rxCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_RX_CHECKSUM_SKIP:
            if (rxCtrl.bitIndex == 0) {
                rxCtrl.state = RF_433_MX_RX_DONE;
                rxCtrl.bitIndex = 7;
            } else {
                rxCtrl.bitIndex--;
            }
            break;

        case RF_433_MX_RX_DONE:
            break;

        default:
            rxCtrl.state  = RF_433_MX_RX_IDLE;
            rxCtrl.synced = 0;
            break;
    }
}

/*==================== 数据读取接口 ====================*/

unsigned char* RF_433_MX_RX_getData(unsigned char *len) {
    if (rxCtrl.state == RF_433_MX_RX_DONE) {
        if (len) *len = rxCtrl.dataLen;
        return rxCtrl.buffer;
    }
    if (len) *len = 0;
    return 0;
}

unsigned char RF_433_MX_RX_isDone(void) {
    return (rxCtrl.state == RF_433_MX_RX_DONE) ? 1 : 0;
}

void RF_433_MX_RX_clearDone(void) {
    if (rxCtrl.state == RF_433_MX_RX_DONE) {
        rxCtrl.state  = RF_433_MX_RX_IDLE;
        rxCtrl.synced = 0;
    }
}

/* 硬件抽象层函数：在平台适配层中实现 */
/* extern 声明见 RF_433_MX_RX.h */
