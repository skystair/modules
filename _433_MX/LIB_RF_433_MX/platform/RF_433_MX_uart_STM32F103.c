/**
 ******************************************************************************
 * @file    RF_433_MX_uart_STM32F103.c
 * @brief   STM32F103C8T6 UART 平台适配层 — 标准库
 * @details
 *   引脚分配：
 *     TX (MX-FS-03V): PA9  - USART1_TX (AF推挽)
 *     RX (MX-05V)   : PA10 - USART1_RX (浮空输入) + EXTI10 下降沿唤醒
 *
 *   唤醒流程：
 *     上电 → AGC噪声（MX-05V输出随机高电平）→ 反相后UART RX = LOW
 *     100ms后 → 使能EXTI10（PA10下降沿）
 *     发射端发0x55前导码 → MX-05V输出HIGH → 反相后PA10出现下降沿
 *     EXTI10触发 → 关闭EXTI → 使能UART RXNE → 后续字节由UART硬件接收
 *
 *   波特率：1200bps（72MHz / 1200 = 60000 → BRR=0xEA60）
 *
 * 依赖：
 *   - dlibx.h（STRUart、Uart_TXIT_func、Uart_RXIT_funcL）
 *   - STM32F10x 标准外设库
 ******************************************************************************
 */

#include "RF_433_MX_uart.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"

#include "dlibx.h"
/*==================== USART1 中断服务函数 ====================*/
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
        Uart_TXIT_func(&uart433);
        USART_ClearITPendingBit(USART1, USART_IT_TXE);
    }

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uart433.Rxtmp = (unsigned char)USART_ReceiveData(USART1);
        Uart_RXIT_funcL(&uart433,uart433.RxDlenPos);
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

/*==================== EXTI15_10 中断服务函数 ====================*/
void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & EXTI_Line10) {
        /* 关闭 EXTI10 中断（一次性，不再需要） */
        EXTI->IMR &= ~EXTI_Line10;
        /* 使能 USART1 接收中断 */
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

        EXTI->PR = EXTI_Line10;    /* 写1清零 */
    }
}

/*==================== 平台层接口实现 ====================*/

void RF_433_MX_uart_sendByte(unsigned char byte) {
    USART_SendData(USART1, (unsigned short)byte);
}

void RF_433_MX_uart_waitTxEnd(void) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void RF_433_MX_uart_txIntEnCtrl(unsigned char en) {
    if (en) {
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    } else {
        USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    }
}

void RF_433_MX_uart_rxEnCtrl(unsigned char en) {
    if (en) {
        USART1->CR1 |= USART_CR1_RE;
    } else {
        USART1->CR1 &= ~USART_CR1_RE;
    }
}

void RF_433_MX_uart_enableWakeup(void) {
    EXTI->PR = EXTI_Line10;    /* 清除可能的残留挂起 */
    EXTI->IMR |= EXTI_Line10;  /* 使能 EXTI10 */
}

/*==================== 平台初始化 ====================*/

static void uart_gpio_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* PA9 = USART1_TX (AF推挽，50MHz) */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA10 = USART1_RX (浮空输入) */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void uart_usart_init(void) {
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /*
     * USART1 在 APB2 上，时钟 = 72 MHz
     * 波特率 1200：BRR = 72000000 / 1200 = 60000 = 0xEA60
     */
    USART_InitStructure.USART_BaudRate            = 1200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    /* NVIC：USART1（RXNE中断初始关闭，由EXTI10唤醒后开启） */
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 使能 USART1 */
    USART_Cmd(USART1, ENABLE);
}

static void uart_exti10_init(void) {
    NVIC_InitTypeDef NVIC_InitStructure;

    /*
     * PA10 → EXTI10（AFIO->EXTICR[2] bits[15:12] = 0000 选择 GPIOA）
     * EXTI10 位于 EXTI15_10_IRQn
     */
    AFIO->EXTICR[2] = (AFIO->EXTICR[2] & ~(0x000Fu << 12)) | (0x0000u << 12);

    /* 配置 EXTI10：下降沿触发 */
    EXTI->IMR  |=  EXTI_Line10;       /* 使能中断 */
    EXTI->FTSR |=  EXTI_Line10;       /* 下降沿触发 */
    EXTI->RTSR &= ~EXTI_Line10;       /* 禁止上升沿 */
    EXTI->PR   =   EXTI_Line10;       /* 清除挂起 */

    /* NVIC：EXTI15_10 */
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 初始不使能EXTI10，等待100ms AGC稳定后由任务层使能 */
    EXTI->IMR &= ~EXTI_Line10;
}

/**
 * 平台层初始化（由 RF_433_MX_uart_init() 内部调用）
 */
void RF_433_MX_uart_platformInit(void) {
    uart_gpio_init();
    uart_usart_init();
    uart_exti10_init();
}
