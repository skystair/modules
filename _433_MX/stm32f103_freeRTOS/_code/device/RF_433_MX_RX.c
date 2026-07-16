#include "RF_433_MX_RX.h"
#include "dlibx.h"
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
/*
 * 由 EXTI 上升沿中断调用
 * 前导码 0xAA 的每个上升沿间隔恰好 1bit (1ms)
 * → 利用上升沿重置 TIM 计数器，实现 bit 级同步
 */
void RF_433_MX_RX_onRisingEdge(void) {
    if (rxCtrl.state == RF_433_MX_RX_IDLE) {
        /* 空闲态检测到上升沿 → 可能是前导码开始 */
        rxCtrl.state         = RF_433_MX_RX_PREAMBLE;
        rxCtrl.preambleCount = 0;
        rxCtrl.bitIndex      = 7;
        rxCtrl.receivedByte  = 0;
        rxCtrl.synced        = 1;
    }
    /* 无论什么状态，都重置超时计数 */
    rxCtrl.timeoutCnt = 0;
}

/*==================== 超时检测 ====================*/
/*
 * 由 TIM 中断每 1ms 调用一次
 * 如果长时间没有边沿触发，说明信号丢失 → 复位状态机
 */
void RF_433_MX_RX_checkTimeout(void) {
    if (rxCtrl.state == RF_433_MX_RX_IDLE) {
        return;
    }
    rxCtrl.timeoutCnt++;
    if (rxCtrl.timeoutCnt >= RF_433_MX_RX_TIMEOUT_MS) {
        /* 超时，复位 */
        rxCtrl.state        = RF_433_MX_RX_IDLE;
        rxCtrl.synced       = 0;
        rxCtrl.timeoutCnt   = 0;
    }
}

/*==================== bit 处理（状态机核心） ====================*/
/*
 * 由 TIM 中断在 bit 中心时刻调用
 * 每次调用处理 1 个 bit，MSB 先发
 */
void RF_433_MX_RX_processBit(unsigned char bitValue) {
    switch (rxCtrl.state) {

        /* ---- 前导码：必须收到 0xAA = 10101010 ---- */
        case RF_433_MX_RX_PREAMBLE: {
            unsigned char expected;
            /*
             * 0xAA = 10101010
             * bitIndex:  7 6 5 4 3 2 1 0
             * expected:  1 0 1 0 1 0 1 0
             *
             * 规律：bitIndex 为奇数时期望 1，偶数时期望 0
             * 即 expected = bitIndex & 1
             */
            expected = rxCtrl.bitIndex & 0x01;
            if (bitValue != expected) {
                /* 不符合前导码模式 → 复位 */
                rxCtrl.state = RF_433_MX_RX_IDLE;
                rxCtrl.synced = 0;
                break;
            }

            if (bitValue) {
                rxCtrl.receivedByte |= (1u << rxCtrl.bitIndex);
            }

            if (rxCtrl.bitIndex == 0) {
                /* 收完 1 字节 */
                if (rxCtrl.receivedByte == 0xAA) {
                    rxCtrl.preambleCount++;
                    if (rxCtrl.preambleCount >= RF_433_MX_RX_PREAMBLE_LEN) {
                        /* 前导码够了 → 转入同步字 */
                        rxCtrl.state        = RF_433_MX_RX_SYNC;
                        rxCtrl.calcChecksum = 0; /* 校验和从同步字开始累加 */
                    }
                } else {
                    /* 字节不对 → 复位 */
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

        /* ---- 同步字：必须收到 0x2D ---- */
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

        /* ---- 数据长度 ---- */
        case RF_433_MX_RX_DATA_LEN:
            if (bitValue) {
                rxCtrl.receivedByte |= (1u << rxCtrl.bitIndex);
            }
            if (rxCtrl.bitIndex == 0) {
                rxCtrl.dataLen = rxCtrl.receivedByte;
                rxCtrl.calcChecksum ^= rxCtrl.dataLen;

                if (rxCtrl.dataLen > RF_433_MX_RX_MAX_DATA_LEN) {
                    /* 长度非法 → 复位 */
                    rxCtrl.state  = RF_433_MX_RX_IDLE;
                    rxCtrl.synced = 0;
                } else if (rxCtrl.dataLen == 0) {
                    /* 无数据 → 直接跳到校验和 */
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

        /* ---- 数据 payload ---- */
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

        /* ---- 校验和 ---- */
        case RF_433_MX_RX_CHECKSUM:
            if (bitValue) {
                rxCtrl.receivedByte |= (1u << rxCtrl.bitIndex);
            }
            if (rxCtrl.bitIndex == 0) {
                rxCtrl.checksum = rxCtrl.receivedByte;
                
//                rxCtrl.calcChecksum = CalSUM_unsign(rxCtrl.buffer, 1,rxCtrl.dataLen);
                
                if (rxCtrl.checksum == rxCtrl.calcChecksum) {
                    rxCtrl.state = RF_433_MX_RX_CHECKSUM_SKIP;  /* 校验通过，跳过第2次校验 */
                } else {
                    rxCtrl.state  = RF_433_MX_RX_IDLE;          /* 校验失败 */
                    rxCtrl.synced = 0;
                }
                rxCtrl.receivedByte = 0;
                rxCtrl.bitIndex = 7;
            } else {
                rxCtrl.bitIndex--;
            }
            break;

        /* ---- 丢弃第2次校验和（冗余字节） ---- */
        case RF_433_MX_RX_CHECKSUM_SKIP:
            /* 仅计数接收，不做任何处理 */
            if (rxCtrl.bitIndex == 0) {
                rxCtrl.state = RF_433_MX_RX_DONE;
                rxCtrl.bitIndex = 7;
            } else {
                rxCtrl.bitIndex--;
            }
            break;

        /* ---- 完成 / 空闲 ---- */
        case RF_433_MX_RX_DONE:
            /* 用户尚未读取，保持 DONE */
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

/* 硬件抽象层函数：在 RF_433_MX_STM32F103.c 中实现 */
/* extern 声明见 RF_433_MX_RX.h */ 
