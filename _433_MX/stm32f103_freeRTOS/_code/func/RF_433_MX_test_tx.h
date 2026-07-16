#ifndef __RF_433_MX_TEST_TX_H__
#define __RF_433_MX_TEST_TX_H__

/* 发射端测试模块
 * 调用方式：
 *   RF_433_MX_test_tx_init()  — 初始化（创建任务前调用一次）
 *   RF_433_MX_test_tx_trig()  — 每次调用发送下一包测试数据
 */

void RF_433_MX_test_tx_init(void);
void RF_433_MX_test_tx_trig(void);

#endif
