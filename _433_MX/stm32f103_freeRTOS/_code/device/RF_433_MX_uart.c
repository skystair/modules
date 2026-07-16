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
/* TX帧缓冲区：sync + len + data + checksum */
unsigned char RF433_txFrameBuf[2 + RF_433_MX_UART_MAX_DATA_LEN + 1];

/* RX帧缓冲区：sync + len + data + checksum（全部存储） */
unsigned char RF433_rxFrameBuf[2 + RF_433_MX_UART_MAX_DATA_LEN + 1];

/* RX头部校验数据：sync + len 占位 */
const unsigned char RF433_rxChkBuf[] = {RF_433_MX_UART_SYNC_WORD,0x77,0x88};

/*==================== 模块实例 ====================*/
STRUart uart433;

/*==================== Watch窗口观察变量 ====================*/
unsigned char rxBuffer[32];    /* 最近一帧数据副本（纯数据，不含头部） */
unsigned char rxLen = 0;       /* 最近一帧数据长度 */
unsigned int  rxPktCount = 0;  /* 累计接收帧数 */

/*==================== RX前导码过滤状态 ====================*/
static unsigned char rxStarted = 0;  /* 0=等待sync 1=已同步，开始接收 */

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
    
    uart433.RenAfterTx  = 1;   /* 发送后重新使能RX */

    uart433.CHKbuff     = RF433_rxChkBuf;
    uart433.Rxbuff      = RF433_rxFrameBuf;
    uart433.RxchkLen    = 3;   /* sync + len 两字节头部校验 */
    uart433.RxLen       = 20;   /* 最小帧长：sync(1)+len(1)+data(0)+checksum(1)+余量 */
    uart433.RxDlenPos   = 3;   
    
//    uart433.fpRXdataGet = 
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

/**
 * 发送一帧数据（阻塞方式）
 *
 * 流程：前导码(0x55×N) → sync(0x2D) → len → data... → checksum
 *
 * @param data  待发送数据指针
 * @param len   数据长度（不含头部和校验）
 * @return 1=成功 0=失败
 */
void RF_433_MX_uart_TXtrig(void){
    uart433.Txtick = 3000;
}
void RF_433_MX_uart_send(void) {
    unsigned char SUM8;
//    /* 1. 发送前导码（阻塞）：唤醒接收端 AGC + 触发 EXTI */
//    for (i = 0; i < RF_433_MX_UART_PREAMBLE_LEN; i++) {
//        RF_433_MX_uart_sendByte(RF_433_MX_UART_PREAMBLE_BYTE);
//    }

//    /* 2. 组装帧到 RF433_txFrameBuf */
//    RF433_txFrameBuf[0] = RF_433_MX_UART_SYNC_WORD;
//    RF433_txFrameBuf[1] = len;

//    checksum = RF_433_MX_UART_SYNC_WORD ^ len;

//    for (i = 0; i < len; i++) {
//        RF433_txFrameBuf[2 + i] = data[i];
//        checksum ^= data[i];
//    }
//    RF433_txFrameBuf[2 + len] = checksum;

//    /* 3. 发送帧（阻塞） */
//    for (i = 0; i < 2 + len + 1; i++) {
//        RF_433_MX_uart_sendByte(RF433_txFrameBuf[i]);
//    }

    if(uart433.Txtick < 3000) return;
        uart433.Txtick = 0;
        if(uart433.TXing) return;
        uart433.TXing = 1;
        uart433.TXp = 0;
        //disen RX
    //    Uart_com_Ren(0);
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
    /* 接收完成后提取数据到 rxBuffer（Watch窗口观察） */
    if (uart433.Rxcompflag) {
        uart433.Rxcompflag = 0;
        unsigned char dataLen = uart433.Rxbuff[3];  /* len字段 */
        unsigned char i;

        if (dataLen <= RF_433_MX_UART_MAX_DATA_LEN) {
            for (i = 0; i < dataLen; i++) {
                rxBuffer[i] = uart433.Rxbuff[4 + i];  /* data从位置2开始 */
            }
            rxLen = dataLen;
            rxPktCount++;
        }

        rxStarted = 0;  /* 准备下一帧 */
    }
}

void RF_433_MX_uart_func(void){
    RF_433_MX_uart_send();
    RF_433_MX_uart_rx_isr();
}