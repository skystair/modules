/**
 ******************************************************************************
 * @file    bldc_drive_6step.c
 * @brief   BLDC 6步换向驱动实现
 ******************************************************************************
 */

#include "config.h"

/* 6步换向表 (根据仿真测试: sector 1~6, state 5,1,3,2,6,4) */
/* 步序: 1=U+V-, 2=U+W-, 3=V+W-, 4=V+U-, 5=W+U-, 6=W+V- */
const Drive_Step_t drive_step_table[7] = {
    /* {uh, ul, vh, vl, wh, wl} */
    {0, 0, 0, 0, 0, 0},  /* index 0: 无效 */
    {1, 0, 0, 1, 0, 0},  /* sector 1: U+ V- */
    {1, 0, 0, 0, 0, 1},  /* sector 2: U+ W- */
    {0, 0, 1, 0, 0, 1},  /* sector 3: V+ W- */
    {0, 1, 1, 0, 0, 0},  /* sector 4: V+ U- */
    {0, 1, 0, 0, 1, 0},  /* sector 5: W+ U- */
    {0, 0, 0, 1, 1, 0},  /* sector 6: W+ V- */
};

/* 全局变量 */
unsigned short g_pwm_duty = 0;
unsigned char g_drive_enable = 0;

/**
 * @brief 初始化 BLDC 驱动引脚
 *        配置 TIM1 为 PWM 输出，使能电源
 */
void BLDC_Drive_Init(void)
{
    /* 使能 GPIO 时钟 */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);

    LL_GPIO_InitTypeDef GPIO_InitStruct;

    /* PA8/PA9/PA10: TIM1_CH1/CH2/CH3 上桥 PWM (复用推挽) */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_8 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;  /* TIM1 */
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB13/PB14/PB15: TIM1_CH1N/CH2N/CH3N 下桥 (复用推挽) */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;  /* TIM1 */
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PB12: TIM1_BKIN 刹车信号 (复用输入) */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PF10: 电源使能脚 (推挽输出，初始化后常开) */
    GPIO_InitStruct.Pin = PM1_CTRL_SD_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(PM1_CTRL_SD_PORT, &GPIO_InitStruct);

    /* 使能电源 */
    BLDC_Drive_Enable();
}

/**
 * @brief 使能驱动 (拉高电源使能脚)
 */
void BLDC_Drive_Enable(void)
{
    LL_GPIO_SetOutputPin(PM1_CTRL_SD_PORT, PM1_CTRL_SD_PIN);
    g_drive_enable = 1;
}

/**
 * @brief 禁用驱动 (拉低电源使能脚)
 */
void BLDC_Drive_Disable(void)
{
    LL_GPIO_ResetOutputPin(PM1_CTRL_SD_PORT, PM1_CTRL_SD_PIN);
    g_drive_enable = 0;
    BLDC_Drive_Stop();
}

/**
 * @brief 设置 PWM 占空比
 * @param duty: 0~1000 (0%=0, 100%=1000)
 */
void BLDC_Drive_SetPWM(unsigned short duty)
{
    if (duty > 1000) duty = 1000;
    g_pwm_duty = duty;

    /* 设置 TIM1 占空比 */
    LL_TIM_OC_SetCompareCH1(TIM1, duty);
    LL_TIM_OC_SetCompareCH2(TIM1, duty);
    LL_TIM_OC_SetCompareCH3(TIM1, duty);
}

/**
 * @brief 执行6步换向
 * @param sector: 扇区号 (1-6)
 */
void BLDC_Drive_Commutation(unsigned char sector)
{
    if (sector > 6 || sector < 1) {
        BLDC_Drive_Stop();
        return;
    }

    if (!g_drive_enable) return;

    const Drive_Step_t *step = &drive_step_table[sector];

    /* U 相控制 */
    if (step->uh) {
        /* U上桥 PWM输出 */
        LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
    } else {
        LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH1);
    }
    if (step->ul) {
        /* U下桥 常开 */
        LL_GPIO_SetOutputPin(PM1_PWM_UL_PORT, PM1_PWM_UL_PIN);
    } else {
        LL_GPIO_ResetOutputPin(PM1_PWM_UL_PORT, PM1_PWM_UL_PIN);
    }

    /* V 相控制 */
    if (step->vh) {
        LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2);
    } else {
        LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH2);
    }
    if (step->vl) {
        LL_GPIO_SetOutputPin(PM1_PWM_VL_PORT, PM1_PWM_VL_PIN);
    } else {
        LL_GPIO_ResetOutputPin(PM1_PWM_VL_PORT, PM1_PWM_VL_PIN);
    }

    /* W 相控制 */
    if (step->wh) {
        LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3);
    } else {
        LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH3);
    }
    if (step->wl) {
        LL_GPIO_SetOutputPin(PM1_PWM_WL_PORT, PM1_PWM_WL_PIN);
    } else {
        LL_GPIO_ResetOutputPin(PM1_PWM_WL_PORT, PM1_PWM_WL_PIN);
    }
}

/**
 * @brief 停止所有输出 (所有桥臂关断)
 */
void BLDC_Drive_Stop(void)
{
    /* 关闭 TIM1 所有通道输出 */
    LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH2);
    LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH3);

    /* 关断所有下桥 */
    LL_GPIO_ResetOutputPin(PM1_PWM_UL_PORT, PM1_PWM_UL_PIN);
    LL_GPIO_ResetOutputPin(PM1_PWM_VL_PORT, PM1_PWM_VL_PIN);
    LL_GPIO_ResetOutputPin(PM1_PWM_WL_PORT, PM1_PWM_WL_PIN);
}
