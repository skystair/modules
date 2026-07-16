/**
 ******************************************************************************
 * @file    dlibx_uart.c
 * @brief   通用 UART 通信模块实现
 * @details
 *   TX 流程：
 *     dlibx_uart_send() → 组装帧到 txBuf → 触发 TXE 中断
 *     → Uart_TXIT_func() 逐字节发送 → 完成后自动切回 RX
 *
 *   RX 流程：
 *     USART RXNE 中断 → Uart_RXIT_funcL() 匹配帧头、变长接收
 *     → 校验通过后置 Rxcompflag → dlibx_uart_func() 提取数据到 rxData
 *
 *   帧格式：[H1] [H2] [H3] [LEN] [DATA...] [CHECKSUM]
 *     TX帧头顺序: TX_H1, TX_H2, TX_H3
 *     RX帧头顺序: RX_H1, RX_H2, RX_H3（与TX不同，用于区分对端）
 ******************************************************************************
 */

#include "dlibx.h"
#include "dlibx_uart.h"
#include <string.h>

/*==================== 内部缓冲区 ====================*/
static unsigned char dlibx_uart_txBuf[3 + 1 + DLIBX_UART_MAX_DATA_LEN + 1]; /* header+len+data+chk */
static unsigned char dlibx_uart_rxBuf[3 + 1 + DLIBX_UART_MAX_DATA_LEN + 1];

/* 帧头校验序列（RX端匹配，顺序与TX不同以区分对端） */
static const unsigned char dlibx_uart_chkBuf[] = {
    DLIBX_UART_RX_H1,
    DLIBX_UART_RX_H2,
    DLIBX_UART_RX_H3,
};

/*==================== 模块实例 ====================*/
STRUart dlibx_uart_str;

/*==================== 接收结果 ====================*/
unsigned char dlibx_uart_rxData[DLIBX_UART_MAX_DATA_LEN];

/*==================== TX 数据组装回调 ====================*/
static void dlibx_uart_txdataSet(void){
    /* 组装帧：[TX_H1][TX_H2][TX_H3][LEN][DATA...][CHK] */
    dlibx_uart_str.TXlen = DLIBX_UART_HEADER_LEN + 1 + DLIBX_UART_TX_LEN_INIT + 1;
    dlibx_uart_str.Txbuff[0] = DLIBX_UART_TX_H1;
    dlibx_uart_str.Txbuff[1] = DLIBX_UART_TX_H2;
    dlibx_uart_str.Txbuff[2] = DLIBX_UART_TX_H3;
    dlibx_uart_str.Txbuff[3] = DLIBX_UART_TX_LEN_INIT;

    /*data*/
}
static void dlibx_uart_rxGet(void){
    
    /* 提取接收数据到全局缓冲区 */
    unsigned char dataLen = dlibx_uart_str.RxLen - DLIBX_UART_RX_DLEN_PLUS;
    if (dataLen <= DLIBX_UART_MAX_DATA_LEN) {
        memcpy(dlibx_uart_rxData, &dlibx_uart_str.Rxbuff[DLIBX_UART_RX_CHK_LEN + 1], dataLen);
    }
}
/*==================== 公共接口 ====================*/

/**
 * 统一初始化入口
 */
void dlibx_uart_init(void) {
    memset(&dlibx_uart_str, 0, sizeof(STRUart));

    /* TX 配置 */
    dlibx_uart_str.Txbuff       = dlibx_uart_txBuf;
    dlibx_uart_str.RenAfterTx   = DLIBX_UART_REN_AFTER_TX;
    dlibx_uart_str.TXlen        = DLIBX_UART_TX_LEN_INIT;
    /* RX 配置 */
    dlibx_uart_str.CHKbuff      = dlibx_uart_chkBuf;
    dlibx_uart_str.Rxbuff       = dlibx_uart_rxBuf;
    dlibx_uart_str.RxchkLen     = DLIBX_UART_RX_CHK_LEN;
    dlibx_uart_str.RxLen        = DLIBX_UART_RX_LEN_INIT;
    dlibx_uart_str.RxDlenPos    = DLIBX_UART_RX_DLEN_POS;
    dlibx_uart_str.RxDlenplus   = DLIBX_UART_RX_DLEN_PLUS;

    /* 平台层函数指针绑定 */
    dlibx_uart_str.fpTxbyte         = dlibx_uart_sendByte;
    dlibx_uart_str.fpWaitTxend      = dlibx_uart_waitTxEnd;
    dlibx_uart_str.fpTxINT_ENctrl   = dlibx_uart_txIntEnCtrl;
    dlibx_uart_str.fpRxen_ENctrl    = dlibx_uart_rxEnCtrl;
    dlibx_uart_str.fpTXdataSet      = dlibx_uart_txdataSet;
    dlibx_uart_str.fpRXdataGet      = dlibx_uart_rxGet;
    /* 清除接收状态 */
    memset(dlibx_uart_rxData, 0, sizeof(dlibx_uart_rxData));

    /* 平台层硬件初始化 */
    dlibx_uart_platformInit();
}

/**
 * 发送一帧数据（通过 txdataSet 回调组装帧内容）
 *
 * TX帧格式：[TX_H1] [TX_H2] [TX_H3] [LEN] [DATA...] [CHECKSUM]
 *
 * @param data  待发送数据指针
 * @param len   数据长度（1~DLIBX_UART_MAX_DATA_LEN）
 * @return 1=触发成功 0=失败
 */
void dlibx_uart_TXfunc(void) {
    unsigned char checksum;
    if (dlibx_uart_str.Txtick < DLIBX_UART_TX_TICK_TRIG) {
        return;
    }
    if (dlibx_uart_str.TXing) {
        return;   /* 忙 */
    }
    dlibx_uart_str.Txtick = 0;
    dlibx_uart_str.fpTXdataSet();  /* 用户可在此回调中修改 txBuf 内容 */
    
    checksum = (unsigned char)CalSUM_unsign(dlibx_uart_str.Txbuff, sizeof(unsigned char), dlibx_uart_str.TXlen-2);
    dlibx_uart_str.Txbuff[dlibx_uart_str.TXlen-1] = checksum;
    /* 设置发送参数 */
    dlibx_uart_str.TXp   = 0;
    dlibx_uart_str.TXing = 1;
    dlibx_uart_str.TXcmp = 0;
    /* 触发发送（使能 TXE 中断） */
    dlibx_uart_str.fpTxINT_ENctrl(1);
}
void dlibx_uart_RXfunc(void) {
    unsigned char checkSum;
    if (dlibx_uart_str.Rxcompflag) {
        dlibx_uart_str.Rxcompflag = 0;
        checkSum = (unsigned char)CalSUM_unsign(dlibx_uart_str.Rxbuff, sizeof(unsigned char), dlibx_uart_str.RxLen-2);
        if(checkSum == dlibx_uart_str.Rxbuff[dlibx_uart_str.RxLen-1]){
            /* 校验成功，处理数据 */
            dlibx_uart_str.fpRXdataGet();
        }else{
            /* 校验失败，丢弃数据 */
        }
    }
}
/**
 * 周期处理函数
 * 从 STRUart 接收缓冲区提取数据到 dlibx_uart_rxData
 */
void dlibx_uart_func(void) {
    dlibx_uart_TXfunc();  /* 触发发送（如果有数据） */
    dlibx_uart_RXfunc();  /* 触发接收（如果有数据） */
}

/*==================== 平台层函数（需由用户实现） ====================*/
/* 参考实现见 RF_433_MX/platform/RF_433_MX_uart_STM32F103.c */
