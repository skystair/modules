/**
 * RF_433_MX_STM32F103.c
 * STM32F103C8T6 平台适配层 — 标准库
 *
 * 引脚分配：
 *   TX (MX-FS-03V): PB15 - 推挽输出
 *   RX (MX-05V)   : PB12 - 浮空输入 + EXTI12 + TIM3 定时采样
 *
 * RX 时序架构:
 *
 *   前导码波形 0xAA:    ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐
 *                       │ │ │ │ │ │ │ │ │ │ │ │ │ │ │ │
 *                     ──┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └──
 *   EXTI 上升沿:        ↑   ↑   ↑   ↑   ↑   ↑   ↑   ↑
 *                        └─重置 TIM3 计数器，同步 bit 时序─┘
 *
 *   TIM3 CC1 (500μs):     ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
 *                         └──在 bit 中心采样 RX 引脚────────┘
 *
 *   TIM3 周期: 1ms (auto-reload=999, prescaler=71 → 72MHz/72=1MHz)
 *   CCR1 比较值: 500 (bit 中心)
 *
 * 注意：EXTI 配置使用直接寄存器操作，不依赖 stm32f10x_exti.c
 *
 * 依赖：
 *   - boardIO.h（需提供 s_boardBSP 和 IO_OUTch_RfTx 枚举）
 *   - STM32F10x 标准外设库
 */

#include "RF_433_MX.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "misc.h"
#include "boardIO.h"
/*==================== 引脚配置 ====================*/
#define RF_TX_GPIO_PORT     GPIOB
#define RF_TX_GPIO_PIN      GPIO_Pin_15
#define RF_TX_GPIO_RCC      RCC_APB2Periph_GPIOB

#define RF_RX_GPIO_PORT     GPIOB
#define RF_RX_GPIO_PIN      GPIO_Pin_12
#define RF_RX_GPIO_RCC      RCC_APB2Periph_GPIOB
#define RF_RX_EXTI_LINE     EXTI_Line12
#define RF_RX_EXTI_IRQ      EXTI15_10_IRQn   /* PB12 → EXTI12，位于 EXTI15_10 中断组 */
#define RF_RX_TIM           TIM3
#define RF_RX_TIM_RCC       RCC_APB1Periph_TIM3
#define RF_RX_TIM_IRQ       TIM3_IRQn

/*==================== 硬件抽象层实现 ====================*/

void RF_433_MX_TX_SetPin(unsigned char level) {
    s_boardBSP.IOout[IO_OUTch_RfTx] = level;
}

void RF_433_MX_TX_DelayUs(unsigned short us) {
    unsigned short i;
    while (us--) {
        i = 10;
        while (i--) { __NOP(); }
    }
}

unsigned char RF_433_MX_RX_GetPin(void) {
    return GPIO_ReadInputDataBit(RF_RX_GPIO_PORT, RF_RX_GPIO_PIN) ? 1 : 0;
}

/**
 * EXTI15_10 中断服务函数
 * 已注释 — UART方案使用 RF_433_MX_uart_STM32F103.c 中的同名ISR
 * */
void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & RF_RX_EXTI_LINE) {
        TIM_SetCounter(RF_RX_TIM, 0);
        RF_433_MX_RX_onRisingEdge();
        EXTI->PR = RF_RX_EXTI_LINE;
    }
}


/**
 * TIM3 中断服务函数
 * 已注释 — UART方案不使用 TIM3 采样
 **/
void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(RF_RX_TIM, TIM_IT_CC1) != RESET) {
        unsigned char bitVal = RF_433_MX_RX_GetPin();
        RF_433_MX_RX_processBit(bitVal);
        TIM_ClearITPendingBit(RF_RX_TIM, TIM_IT_CC1);
    }
    if (TIM_GetITStatus(RF_RX_TIM, TIM_IT_Update) != RESET) {
        RF_433_MX_RX_checkTimeout();
        TIM_ClearITPendingBit(RF_RX_TIM, TIM_IT_Update);
    }
}


/*==================== 平台初始化 ====================*/

/* GPIO 初始化已移至 boardIO.c 的 Board_Init() */

/**
 * EXTI 初始化 — PB12 上升沿触发
 * AFIO->EXTICR[3] bits[3:0] = 0x0001 选择 GPIOB
 */
static void RF_433_MX_EXTI_Init(void) {
    NVIC_InitTypeDef NVIC_InitStructure;

    /* PB12 → EXTI12 (AFIO->EXTICR[3] bits[3:0] = 0001 选择 GPIOB) */
    AFIO->EXTICR[3] = (AFIO->EXTICR[3] & ~(0x000Fu << 0)) | (0x0001u << 0);

    /* 配置 EXTI12：上升沿触发 */
    EXTI->IMR  |=  RF_RX_EXTI_LINE;       /* 使能中断 */
    EXTI->FTSR &= ~RF_RX_EXTI_LINE;       /* 禁止下降沿 */
    EXTI->RTSR |=  RF_RX_EXTI_LINE;       /* 上升沿触发 */
    EXTI->PR   =   RF_RX_EXTI_LINE;       /* 清除挂起 */

    /* NVIC：EXTI15_10 */
    NVIC_InitStructure.NVIC_IRQChannel                   = RF_RX_EXTI_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * TIM3 初始化 — 500μs 中心采样
 * 时钟源: APB1=36MHz ×2 = 72MHz
 * Prescaler=71 → 72MHz/72 = 1MHz (1μs)
 * Auto-reload=999 → 周期 1ms
 * CCR1=500 → 每 bit 中心触发 CC1 中断
 */
static void RF_433_MX_TIM_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;

    /* 使能 TIM3 时钟 */
    RCC_APB1PeriphClockCmd(RF_RX_TIM_RCC, ENABLE);

    /* 时基配置 */
    TIM_TimeBaseStructure.TIM_Prescaler         = 71;               /* 72MHz/72 = 1MHz */
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period            = 999;              /* 1MHz/1000 = 1kHz (1ms) */
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(RF_RX_TIM, &TIM_TimeBaseStructure);

    /* CC1 输出比较配置 — 比较值 500 (bit 中心) */
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_Pulse        = 500;
    TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High;
    TIM_OC1Init(RF_RX_TIM, &TIM_OCInitStructure);

    /* NVIC：TIM3 */
    NVIC_InitStructure.NVIC_IRQChannel                   = RF_RX_TIM_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 使能 CC1 + 更新中断 */
    TIM_ITConfig(RF_RX_TIM, TIM_IT_CC1 | TIM_IT_Update, ENABLE);

    /* 使能 TIM3 */
    TIM_Cmd(RF_RX_TIM, ENABLE);
}

/**
 * STM32F103 平台硬件初始化
 **/
void RF_433_MX_PlatformInit(void) {
    RF_433_MX_EXTI_Init();
    RF_433_MX_TIM_Init();
}
