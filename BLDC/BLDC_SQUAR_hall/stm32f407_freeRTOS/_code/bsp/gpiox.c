#include "config.h"

/* 输出引脚状态数组 */
unsigned char IOoutCtrl[IO_OUTch_Max];

/* 输入引脚状态数组 */
unsigned char IOinRead[IO_INch_Max];

/**
 * @brief GPIO 初始化
 */
void gpiox_init(void)
{
    /* 使能 GPIO 时钟 */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);   /* HALL 传感器 */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);   /* LED + 按键 */

    LL_GPIO_InitTypeDef GPIO_InitStruct;

        /* LED 默认灭 */
    LL_GPIO_SetOutputPin(GPIOE, LED0_PIN | LED1_PIN);
//    LL_GPIO_ResetOutputPin(GPIOE, LED0_PIN);
    
    /* 霍尔传感器输入: PH10, PH11, PH12 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
    LL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
    LL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    /* 按键输入: PE2, PE3, PE4 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* LED 输出: PE0, PE1 */
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;

    GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
    IOoutCtrl[IO_OUTch_LED0] = 1;
}

/**
 * @brief GPIO 周期性处理函数
 *        - 读取霍尔传感器状态
 *        - 更新按键状态
 */
void gpiox_func(void)
{
    /* 读取霍尔传感器状态 */
    IOinRead[IO_INch_BLDC_hallU] = LL_GPIO_IsInputPinSet(HALL_U_PORT, HALL_U_PIN);
    IOinRead[IO_INch_BLDC_hallV] = LL_GPIO_IsInputPinSet(HALL_V_PORT, HALL_V_PIN);
    IOinRead[IO_INch_BLDC_hallW] = LL_GPIO_IsInputPinSet(HALL_W_PORT, HALL_W_PIN);
    
    
    /* LED 控制 */
    if (!IOoutCtrl[IO_OUTch_LED0]) {
        LL_GPIO_SetOutputPin(LED0_PORT, LED0_PIN);
    } else {
        LL_GPIO_ResetOutputPin(LED0_PORT, LED0_PIN);
    }

    if (!IOoutCtrl[IO_OUTch_LED1]) {
        LL_GPIO_SetOutputPin(LED1_PORT, LED1_PIN);
    } else {
        LL_GPIO_ResetOutputPin(LED1_PORT, LED1_PIN);
    }
}