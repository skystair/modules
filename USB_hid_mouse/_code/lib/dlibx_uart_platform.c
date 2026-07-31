/**
 ******************************************************************************
 * @file    dlibx_uart_platform.c
 * @brief   dlibx_uart 平台适配参考实现
 * @details
 *   本文件为 dlibx_uart 的平台层参考模板，展示如何对接具体 MCU 的 UART 外设。
 *   实际使用时：
 *     1. 复制本文件，去掉函数体注释，填入实际引脚和外设配置
 *     2. 根据硬件修改 GPIO/USART/引脚 宏定义
 *     3. 在工程中添加本文件和 dlibx_uart.c、dlibx.c
 *
 *   参考引脚分配（以 USART1 为例，可根据实际硬件修改）：
 *     TX: PA9  - USART1_TX (AF推挽)
 *     RX: PA10 - USART1_RX (浮空输入)
 *
 * 依赖：
 *   - dlibx.h（STRUart、Uart_TXIT_func、Uart_RXIT_funcL）
 *   - dlibx_uart.h（dlibx_uart_str 实例声明）
 *   - 具体 MCU 的 HAL/标准库
 ******************************************************************************
 */

#include "dlibx_uart.h"
#include "dlibxConf.h"

/* MCU 头文件（按需取消注释，替换为实际平台库） */
// #include "stm32f10x.h"
// #include "stm32f10x_gpio.h"
// #include "stm32f10x_rcc.h"
// #include "stm32f10x_usart.h"
// #include "misc.h"

/*==================== 硬件配置宏（根据实际硬件修改） ====================*/
// #define DLIBX_UART_USART        USART1
// #define DLIBX_UART_USART_IRQn   USART1_IRQn
// #define DLIBX_UART_TX_PORT      GPIOA
// #define DLIBX_UART_TX_PIN       GPIO_Pin_9
// #define DLIBX_UART_RX_PORT      GPIOA
// #define DLIBX_UART_RX_PIN       GPIO_Pin_10
// #define DLIBX_UART_GPIO_RCC     RCC_APB2Periph_GPIOA
// #define DLIBX_UART_USART_RCC    RCC_APB2Periph_USART1
// #define DLIBX_UART_BAUD         9600

/*==================== USART 中断服务函数（参考） ====================*/
/**
 * @brief USART 中断服务函数
 * @note  需在中断向量文件或本文件中实现，根据实际使用的 USART 编号修改函数名
 *       USART1 → USART1_IRQHandler
 *       USART2 → USART2_IRQHandler
 *       USART3 → USART3_IRQHandler
 */
// void USART1_IRQHandler(void) {
//     /* TXE 中断：逐字节发送 */
//     if (USART_GetITStatus(DLIBX_UART_USART, USART_IT_TXE) != RESET) {
//         Uart_TXIT_func(&dlibx_uart_str);
//         USART_ClearITPendingBit(DLIBX_UART_USART, USART_IT_TXE);
//     }
//
//     /* RXNE 中断：逐字节接收 */
//     if (USART_GetITStatus(DLIBX_UART_USART, USART_IT_RXNE) != RESET) {
//         dlibx_uart_str.Rxtmp = (unsigned char)USART_ReceiveData(DLIBX_UART_USART);
//         Uart_RXIT_funcL(&dlibx_uart_str, dlibx_uart_str.RxDlenPos);
//         USART_ClearITPendingBit(DLIBX_UART_USART, USART_IT_RXNE);
//     }
// }

/*==================== 平台层接口实现 ====================*/

/**
 * @brief 发送1字节（写入 USART 数据寄存器）
 * @note  由 Uart_TXIT_func() 在 TXE 中断中调用
 */
void dlibx_uart_sendByte(unsigned char byte) {
//    USART_SendData(DLIBX_UART_USART, (unsigned short)byte);
}

/**
 * @brief 等待发送完成（TC + TXE 标志）
 * @note  由 Uart_TXIT_func() 在最后一字节发送后调用
 */
void dlibx_uart_waitTxEnd(void) {
//    while (USART_GetFlagStatus(DLIBX_UART_USART, USART_FLAG_TC) == RESET);
//    while (USART_GetFlagStatus(DLIBX_UART_USART, USART_FLAG_TXE) == RESET);
}

/**
 * @brief TX 中断使能控制
 * @param en  1=使能 TXE 中断  0=关闭 TXE 中断
 */
void dlibx_uart_txIntEnCtrl(unsigned char en) {
//    if (en) {
//        USART_ITConfig(DLIBX_UART_USART, USART_IT_TXE, ENABLE);
//    } else {
//        USART_ITConfig(DLIBX_UART_USART, USART_IT_TXE, DISABLE);
//    }
}

/**
 * @brief RX 使能控制（RE 位）
 * @param en  1=使能接收器  0=关闭接收器
 */
void dlibx_uart_rxEnCtrl(unsigned char en) {
//    if (en) {
//        DLIBX_UART_USART->CR1 |= USART_CR1_RE;
//    } else {
//        DLIBX_UART_USART->CR1 &= ~USART_CR1_RE;
//    }
}

/*==================== 平台初始化 ====================*/

// static void dlibx_uart_gpio_init(void) {
//     GPIO_InitTypeDef GPIO_InitStructure;
//
//     RCC_APB2PeriphClockCmd(DLIBX_UART_GPIO_RCC, ENABLE);
//     RCC_APB2PeriphClockCmd(DLIBX_UART_USART_RCC, ENABLE);
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//
//     /* TX 引脚：AF推挽，50MHz */
//     GPIO_InitStructure.GPIO_Pin   = DLIBX_UART_TX_PIN;
//     GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(DLIBX_UART_TX_PORT, &GPIO_InitStructure);
//
//     /* RX 引脚：浮空输入 */
//     GPIO_InitStructure.GPIO_Pin   = DLIBX_UART_RX_PIN;
//     GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
//     GPIO_Init(DLIBX_UART_RX_PORT, &GPIO_InitStructure);
// }

// static void dlibx_uart_usart_init(void) {
//     USART_InitTypeDef USART_InitStructure;
//     NVIC_InitTypeDef  NVIC_InitStructure;
//
//     /* USART 时基配置 */
//     USART_InitStructure.USART_BaudRate            = DLIBX_UART_BAUD;
//     USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
//     USART_InitStructure.USART_StopBits            = USART_StopBits_1;
//     USART_InitStructure.USART_Parity              = USART_Parity_No;
//     USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//     USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
//     USART_Init(DLIBX_UART_USART, &USART_InitStructure);
//
//     /* NVIC 配置 */
//     NVIC_InitStructure.NVIC_IRQChannel                   = DLIBX_UART_USART_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
//     NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
//     NVIC_Init(&NVIC_InitStructure);
//
//     /* 使能 RXNE 中断（初始即可接收，无需等待唤醒） */
//     USART_ITConfig(DLIBX_UART_USART, USART_IT_RXNE, ENABLE);
//
//     /* 使能 USART */
//     USART_Cmd(DLIBX_UART_USART, ENABLE);
// }

/**
 * @brief 平台层初始化（由 dlibx_uart_init() 内部调用）
 * @note  完成 GPIO、USART、NVIC 初始化
 */
void dlibx_uart_platformInit(void) {
//    dlibx_uart_gpio_init();
//    dlibx_uart_usart_init();
}
