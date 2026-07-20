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

/*==================== 默认配置 ====================*/
static const unsigned char uart_exp1_txHeader[] = {
    DLIBX_UART_TX_H1,
    DLIBX_UART_TX_H2,
    DLIBX_UART_TX_H3,
};

static const unsigned char uart_exp1_rxHeader[] = {
    DLIBX_UART_RX_H1,
    DLIBX_UART_RX_H2,
    DLIBX_UART_RX_H3,
};

/* default buffers for the default config */
static unsigned char uart_exp1_txBuf[DLIBX_UART_MAX_DATA_LEN + 1 + 1 + 3];
static unsigned char uart_exp1_rxBuf[DLIBX_UART_MAX_DATA_LEN + 1 + 1 + 3];
static unsigned char uart_exp1_rxData[DLIBX_UART_MAX_DATA_LEN];

/*==================== TX/RX 数据处理回调 ====================*/
static void dlibx_uart_txDataSet(STRUart* uart) {
    const dlibx_uart_cfg_t* cfg = (uart != 0) ? uart->cfg : 0;
    unsigned char index;
    unsigned char txHeaderLen = (cfg != 0) ? cfg->txHeaderLen : 0;
    unsigned char txDataLen = (cfg != 0) ? cfg->txLenInit : 0;

    /* 组装帧：[TX_H1] [TX_H2] ... [LEN] [DATA...] [CHK] */
    uart->TXlen = (unsigned short)(txHeaderLen + 1 + txDataLen + 1);
    for (index = 0; index < txHeaderLen; ++index) {
        uart->Txbuff[index] = cfg->txHeader[index];
    }
    uart->Txbuff[txHeaderLen] = txDataLen;
}

static void dlibx_uart_rxGet(STRUart* uart) {
    const dlibx_uart_cfg_t* cfg = (uart != 0) ? uart->cfg : 0;
    unsigned char dataLen;
    unsigned char rxHeaderLen = (unsigned char)uart->RxchkLen;

    /* 提取接收数据到实例的 rxData 缓冲区 */
    dataLen = (unsigned char)(uart->RxLen - uart->RxDlenplus);
    if (dataLen > DLIBX_UART_MAX_DATA_LEN) {
        dataLen = DLIBX_UART_MAX_DATA_LEN;
    }

    if (dataLen > 0) {
        if (uart->rxData) {
            unsigned char copyLen = dataLen;
            if (cfg && copyLen > uart->rxDataMaxLen) copyLen = uart->rxDataMaxLen;
            memcpy(uart->rxData, &uart->Rxbuff[rxHeaderLen + 1], copyLen);
            uart->rxDataLen = copyLen;
        }
    } else {
        if (uart->rxData) {
            memset(uart->rxData, 0, uart->rxDataMaxLen);
            uart->rxDataLen = 0;
        }
    }
}
/*==================== 模块实例（cfg 存放静态配置，实例在声明处只绑定 cfg 指针） ====================*/
static const dlibx_uart_cfg_t uart_exp1_cfg = {
    uart_exp1_txBuf,
    uart_exp1_rxBuf,
    uart_exp1_txHeader,
    uart_exp1_rxHeader,
    (unsigned char)(sizeof(uart_exp1_txHeader) / sizeof(uart_exp1_txHeader[0])),
    (unsigned char)(sizeof(uart_exp1_rxHeader) / sizeof(uart_exp1_rxHeader[0])),
    UART_EXP1_RX_LEN_INIT,
    UART_EXP1_TX_LEN_INIT,
    UART_EXP1_REN_AFTER_TX,
    UART_EXP1_TX_TICK_TRIG,

    uart_exp1_rxData,
    DLIBX_UART_MAX_DATA_LEN,

    dlibx_uart_sendByte,
    dlibx_uart_waitTxEnd,
    dlibx_uart_txIntEnCtrl,
    dlibx_uart_rxEnCtrl,

    dlibx_uart_txDataSet,
    dlibx_uart_rxGet
};

/* 实例仅在声明处绑定 cfg 指针，子成员由 initEx 从 cfg 应用（或可在声明处某些字段显式覆盖） */
STRUart uart_exp1 = {
    /* cfg pointer */
    .cfg = &uart_exp1_cfg
};

/**
 * 发送一帧数据（通过 txDataSet 回调组装帧内容）
 *
 * TX帧格式：[TX_H1] [TX_H2] [TX_H3] [LEN] [DATA...] [CHECKSUM]
 */
void dlibx_uart_TXfunc(STRUart* uart) {
    unsigned char checksum;

    if (uart == 0) return;

    /* use per-instance trigger configured in cfg (fall back to global macro if not set) */
    {
        unsigned char trig = (uart->cfg && uart->cfg->txTickTrig) ? uart->cfg->txTickTrig : DLIBX_UART_TX_TICK_TRIG;
        if (uart->Txtick < trig) {
            return;
        }
    }
    if (uart->TXing) {
        return;   /* 忙 */
    }

    uart->Txtick = 0;
    if (uart->fpTXdataSet) uart->fpTXdataSet(uart);  /* 用户可在此回调中修改 txBuf 内容 */

    checksum = (unsigned char)CalSUM_unsign(uart->Txbuff, sizeof(unsigned char), uart->TXlen - 2);
    uart->Txbuff[uart->TXlen - 1] = checksum;

    /* 设置发送参数 */
    uart->TXp   = 0;
    uart->TXing = 1;
    uart->TXcmp = 0;

    /* 触发发送（使能 TXE 中断） */
    if (uart->fpTxINT_ENctrl) uart->fpTxINT_ENctrl(1);
}

void dlibx_uart_RXfunc(STRUart* uart) {
    unsigned char checkSum;

    if (uart == 0) return;

    if (uart->Rxcompflag) {
        uart->Rxcompflag = 0;
        checkSum = (unsigned char)CalSUM_unsign(uart->Rxbuff, sizeof(unsigned char), uart->RxLen - 2);
        if (checkSum == uart->Rxbuff[uart->RxLen - 1]) {
            /* 校验成功，处理数据 */
            if (uart->fpRXdataGet) uart->fpRXdataGet(uart);
        } else {
            /* 校验失败，丢弃数据 */
        }
    }
}

/*==================== 平台层函数（需由用户实现） ====================*/
/**
 * 使用指定配置初始化
 */
void dlibx_uart_initEx(STRUart* uart) {
    if (uart == 0) {
        return;
    }

    /* 保存将要使用的 cfg 指针（优先使用传入 cfg，否则使用实例当前绑定的 cfg） */
    const dlibx_uart_cfg_t* use_cfg = uart->cfg;
    if (use_cfg == NULL) return; /* 最后回退到默认 cfg */

    /* 将实例内存清零，然后把非零运行时字段统一赋值（减少分支） */
    memset(uart, 0, sizeof(STRUart));

    /* 先应用 cfg 指针（cfg 为 const，实例保存指向 cfg 的指针） */
    uart->cfg = use_cfg;

    /* 使用 cfg 里的静态配置来初始化运行时字段 */
    uart->TXlen = (unsigned short int)(use_cfg->txHeaderLen + 1 + use_cfg->txLenInit + 1);
    uart->RxLen = use_cfg->rxLenInit;
    uart->RxchkLen = use_cfg->rxHeaderLen;
    uart->RxDlenPos = use_cfg->rxHeaderLen;
    uart->RxDlenplus = use_cfg->rxHeaderLen + 1 + 1;
    uart->RenAfterTx = use_cfg->renAfterTx;

    /* 运行时缓冲区指针：优先使用 cfg 中提供的缓冲；若 cfg 未提供且这是默认实例，则退回到模块内的默认缓冲 */
    uart->Txbuff = use_cfg->txBuf;
    uart->Rxbuff = use_cfg->rxBuf
    uart->CHKbuff = use_cfg->rxHeader;

    /* 将 cfg 中的接收结果缓冲指针与大小应用到实例 */
    uart->rxData = use_cfg->rxData;
    uart->rxDataMaxLen = use_cfg->rxDataMaxLen;
    uart->rxDataLen = 0;

    /* 覆盖默认回调：如果 cfg 提供了回调，则使用 cfg 的实现 */
    if (use_cfg->fpTxbyte) uart->fpTxbyte = use_cfg->fpTxbyte;
    if (use_cfg->fpWaitTxend) uart->fpWaitTxend = use_cfg->fpWaitTxend;
    if (use_cfg->fpTxINT_ENctrl) uart->fpTxINT_ENctrl = use_cfg->fpTxINT_ENctrl;
    if (use_cfg->fpRxEnCtrl) uart->fpRxen_ENctrl = use_cfg->fpRxEnCtrl;
    if (use_cfg->fpTXdataSet) uart->fpTXdataSet = use_cfg->fpTXdataSet;
    if (use_cfg->fpRXdataGet) uart->fpRXdataGet = use_cfg->fpRXdataGet;
}

void dlibx_uart_init(void) {
    /* 默认实例已在声明处静态初始化；此处仅执行平台初始化 */
    dlibx_uart_platformInit();//所有实例的平台初始化
    
    dlibx_uart_initEx(&uart_exp1);
}

/**
 * 周期处理函数
 * 从 STRUart 接收缓冲区提取数据；接收的 DATA 存放到 uart->rxData，长度写入 uart->rxDataLen
 */
void dlibx_uart_func(void) {
    /* 兼容主循环轮询：默认轮询 uart_exp1 实例 */
    dlibx_uart_TXfunc(&uart_exp1);
    dlibx_uart_RXfunc(&uart_exp1);
}