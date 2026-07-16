/**
 ******************************************************************************
 * @file    RF_433_MX_uart.c
 * @brief   433MHz OOK模块 UART协议层实现（参照Uart_RXIT_funcL模式）
 * @details
 *   RX: 前导码过滤 → sync字节匹配 → Uart_RXIT_funcL变长接收
 *   TX: 阻塞发送前导码 → sync → len → data → checksum
 *
 * 帧格式：  [0x55 × N preamble] [0x2D sync] [LEN] [DATA...] [CHECKSUM]
 * 校验和：  CHECKSUM = SYNC ^ LEN ^ DATA[0] ^ ... ^ DATA[N-1]
 ******************************************************************************
 */
#include "dlibx.h"
#include "RF_433_MX_uart.h"
#include <string.h>

/*==================== 内部缓冲区 ====================*/
unsigned char RF433_txFrameBuf[2 + RF_433_MX_UART_MAX_DATA_LEN + 1];
unsigned char RF433_rxFrameBuf[2 + RF_433_MX_UART_MAX_DATA_LEN + 1];

const unsigned char RF433_rxChkBuf[] = {RF_433_MX_UART_SYNC_WORD,0x77,0x88};

/*==================== 模块实例 ====================*/
STRUart uart433;

/*==================== Watch窗口观察变量 ====================*/
unsigned char rxBuffer[32];
unsigned char rxLen = 0;
unsigned int  rxPktCount = 0;

/*==================== RX前导码过滤状态 ====================*/
static unsigned char rxStarted = 0;

/*==================== 公共接口 ====================*/
static void tx_dataSet(void){
    uart433.Txbuff[0] = RF_433_MX_UART_SYNC_WORD;
    uart433.Txbuff[1] = 0x77;
    uart433.Txbuff[2] = 0x88;
    uart433.Txbuff[3] = 0x05;
    uart433.Txbuff[4] = 0x01;
    uart433.Txbuff[5] = 0x02;
    uart433.Txbuff[6] = 0x03;
    uart433.Txbuff[7] = 0x04;
    uart433.Txbuff[8] = 0x05;

    uart433.TXlen = uart433.Txbuff[3] + 5;
}

/**
 * 统一初始化入口
 */
void RF_433_MX_uart_init(void) {
    memset(&uart433, 0, sizeof(STRUart));
    /* TX 配置 */
    uart433.Txbuff      = RF433_txFrameBuf;

    uart433.RenAfterTx  = 1;

    uart433.CHKbuff     = RF433_rxChkBuf;
    uart433.Rxbuff      = RF433_rxFrameBuf;
    uart433.RxchkLen    = 3;
    uart433.RxLen       = 20;
    uart433.RxDlenPos   = 3;

    uart433.fpRxen_ENctrl = RF_433_MX_uart_rxEnCtrl;
    uart433.fpTxbyte = RF_433_MX_uart_sendByte;
    uart433.fpTXdataSet = tx_dataSet;
    uart433.fpTxINT_ENctrl = RF_433_MX_uart_txIntEnCtrl;
    uart433.fpWaitTxend = RF_433_MX_uart_waitTxEnd;

    rxStarted = 0;
    rxLen     = 0;
    rxPktCount = 0;
    memset(rxBuffer, 0, sizeof(rxBuffer));

    /* 平台层初始化（GPIO + USART + EXTI） */
    RF_433_MX_uart_platformInit();
}

void RF_433_MX_uart_TXtrig(void){
    uart433.Txtick = 3000;
}
void RF_433_MX_uart_send(void) {
    unsigned char SUM8;

    if(uart433.Txtick < 3000) return;
        uart433.Txtick = 0;
        if(uart433.TXing) return;
        uart433.TXing = 1;
        uart433.TXp = 0;
        //prepare seled tx data
        uart433.fpTXdataSet();
        SUM8 = (unsigned char)GetSUM(uart433.Txbuff,0,uart433.TXlen-2);

        uart433.Txbuff[uart433.TXlen-1] = SUM8;
        //en seled tx
        uart433.fpTxINT_ENctrl(1);
}

/**
 * 接收ISR入口（平台层RXNE中断调用）
 */
void RF_433_MX_uart_rx_isr(void){
    if (uart433.Rxcompflag) {
        uart433.Rxcompflag = 0;
        unsigned char dataLen = uart433.Rxbuff[3];
        unsigned char i;

        if (dataLen <= RF_433_MX_UART_MAX_DATA_LEN) {
            for (i = 0; i < dataLen; i++) {
                rxBuffer[i] = uart433.Rxbuff[4 + i];
            }
            rxLen = dataLen;
            rxPktCount++;
        }

        rxStarted = 0;
    }
}

void RF_433_MX_uart_func(void){
    RF_433_MX_uart_send();
    RF_433_MX_uart_rx_isr();
}
