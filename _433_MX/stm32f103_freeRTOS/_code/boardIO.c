/**
 ******************************************************************************
 * @file    boardIO.c
 * @brief   STM32F103C8T6 底层硬件配置实现
 * @details 根据 boardIO.h 中的枚举 + const 结构体数组，
 *          for 循环统一初始化所有 GPIO 引脚
 ******************************************************************************
 */
#include "config.h"

/* ============================================================================
 *                        GPIO 配置数组
 * ============================================================================ */
/**
 * @brief GPIO 输出引脚配置数组
 * @note  索引对应 io_out_ch_t 枚举
 */
const BoardGpioOutConfig_t Board_GpioOutputs[IO_OUTch_Max] = {
    /* IO_OUTch_Led     */ { GPIOA, GPIO_Pin_8,  0 },
    /* IO_OUTch_RfTx    */ { GPIOB, GPIO_Pin_15, 0 },
};

/**
 * @brief GPIO 输入引脚配置数组
 * @note  索引对应 io_in_ch_t 枚举
 */
const BoardGpioInConfig_t Board_GpioInputs[IO_INch_Max] = {
    /* IO_INch_Key  */ { GPIOC, GPIO_Pin_13 ,0},
    /* IO_INch_RfRx */ { GPIOB, GPIO_Pin_12 ,1},
};

/* ============================================================================
 *                        GPIO 初始化
 * ============================================================================ */

/**
 * @brief 按端口分组初始化所有输出引脚
 * @note  同一端口的引脚合并为一次 GPIO_Init 调用
 */
static void Board_OutputInit(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    for(unsigned char i = 0;i < IO_OUTch_Max;i++){
        if (Board_GpioOutputs[i].defaultLvl) {
            GPIO_SetBits(Board_GpioOutputs[i].port, Board_GpioOutputs[i].pin);
        } else {
            GPIO_ResetBits(Board_GpioOutputs[i].port, Board_GpioOutputs[i].pin);
        }
        GPIO_InitStructure.GPIO_Pin = Board_GpioOutputs[i].pin;
        GPIO_Init(Board_GpioOutputs[i].port, &GPIO_InitStructure);
    }
}

/**
 * @brief for 循环初始化所有输入引脚
 */
static void Board_InputInit(void) {
    unsigned char i;
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

    for (i = 0; i < IO_INch_Max; i++) {
        GPIO_InitStructure.GPIO_Pin = Board_GpioInputs[i].pin;
        GPIO_Init(Board_GpioInputs[i].port, &GPIO_InitStructure);
    }
}

/* ============================================================================
 *                        板级初始化入口
 * ============================================================================ */

void Board_Init(void) {
    /* 使能所有用到的 GPIO 端口时钟 + AFIO */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC |
        RCC_APB2Periph_AFIO,
        ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* for 循环初始化 */
    Board_OutputInit();
    Board_InputInit();
}

BoardBSPConfig_t s_boardBSP;
void Board_BSPfunc(void){
    unsigned char i;
    //in
    for(i = 0;i < IO_INch_Max;i++){
        s_boardBSP.IOin[i] = (Board_GpioInputs[i].defaul !=
        GPIO_ReadInputDataBit(Board_GpioInputs[i].port,Board_GpioInputs[i].pin));
    }
    //out
    for(i = 0;i < IO_OUTch_Max;i++){
        if (s_boardBSP.IOout[i]) {
            GPIO_SetBits(Board_GpioOutputs[i].port, Board_GpioOutputs[i].pin);
        } else {
            GPIO_ResetBits(Board_GpioOutputs[i].port, Board_GpioOutputs[i].pin);
        }
    }
    //
}
