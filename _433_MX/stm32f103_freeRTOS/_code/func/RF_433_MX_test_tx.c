/**
 * RF_433_MX_test_tx.c
 * 发射端测试用例 — 封装版
 *
 * 测试数据：
 *   包1: {0x01, 0x02, 0x03, 0x04, 0x05}
 *   包2: {0xAA, 0x55, 0xFF}
 *   包3: {0xDE, 0xAD, 0xBE, 0xEF}
 *
 * 使用：
 *   RF_433_MX_test_tx_init();   // 初始化一次
 *   RF_433_MX_test_tx_trig();   // 按键时调用，发送下一包
 */

#include "config.h"

#if 1  /* UART方案启用时，屏蔽旧测试代码 */

#include "RF_433_MX_test_tx.h"

/*==================== 测试数据 ====================*/
#define TEST_PKT_COUNT  10
unsigned char testbuff[TEST_PKT_COUNT] =  {0x01, 0x02, 0x03, 0x04, 0x05};

typedef struct {
    unsigned char *data;
    unsigned char len;
} TestPacket;

static unsigned char pkt1[] = {0x01, 0x02, 0x03, 0x04, 0x05};   //0x0f
static unsigned char pkt2[] = {0xAA, 0x55, 0xFF};               //0xfe
static unsigned char pkt3[] = {0xDE, 0xAD, 0xBE, 0xEF};         //0x38


TestPacket testPkts = {
    testbuff, 5
};

static unsigned char pktIndex = 0;

/*==================== 接口函数 ====================*/

void RF_433_MX_test_tx_init(void) {
    pktIndex = 0;
}

void RF_433_MX_test_tx_trig(void) {
    if (!RF_433_MX_TX_isIdle()) return;

    RF_433_MX_TX_send(testPkts.data,
                      testPkts.len);
    LEDxCtrl(LED_CH_W, LED_STATE_FLASH, 100);

    pktIndex++;
    if (pktIndex >= TEST_PKT_COUNT) {
        pktIndex = 0;
    }
}

#endif  /* UART方案：屏蔽旧测试代码 */
