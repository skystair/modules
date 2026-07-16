#ifndef __RF_433_MX_TEST_RX_H__
#define __RF_433_MX_TEST_RX_H__

/* 接收端测试模块
 * 调用方式：
 *   RF_433_MX_test_rx_init()  — 初始化（创建任务前调用一次）
 *   RF_433_MX_test_rx_poll()  — 周期调用，内部轮询 isDone()
 *
 * 调试：Watch 窗口查看 rxBuffer[0..rxLen-1] 和 rxPktCount
 */

#include "RF_433_MX_RX.h"

void RF_433_MX_test_rx_init(void);
void RF_433_MX_test_rx_poll(void);

/* 只读调试变量（外部可观察） */
extern unsigned char rxBuffer[RF_433_MX_RX_MAX_DATA_LEN];
extern unsigned char rxLen;
extern unsigned int  rxPktCount;

#endif
