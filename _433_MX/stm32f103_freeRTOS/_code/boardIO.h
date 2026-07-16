#ifndef __BOARDIO_H
#define __BOARDIO_H

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

/* ============================================================================
 * @file    boardIO.h
 * @brief   STM32F103C8T6 底层硬件配置头文件
 * @details 集中定义 MCU 物理引脚映射，不涉及具体业务逻辑
 *          - GPIO 输出 / 输入均通过枚举索引 + const 结构体数组描述
 *          - Board_Init() 中 for 循环统一初始化
 *          参照 board_v1 模式
 * ============================================================================ */

/* ===========================================================================
 *                          GPIO 输出通道
 * =========================================================================== */
typedef enum {
    IO_OUTch_Led = 0,          /* LED 指示灯 (PA9) */
    IO_OUTch_RfTx,             /* RF 433MHz 发射 (PB15) */

    IO_OUTch_Max               /* 输出通道总数 */
} io_out_ch_t;

typedef struct {
    GPIO_TypeDef *port;        /* GPIO 端口 (GPIOA, GPIOB, ...) */
    unsigned short pin;        /* GPIO 引脚 (GPIO_Pin_x) */
    unsigned char  defaultLvl; /* 默认电平 (0=低, 1=高) */
} BoardGpioOutConfig_t;

/* ===========================================================================
 *                          GPIO 输入通道
 * =========================================================================== */
typedef enum {
    IO_INch_Key = 0,           /* 按键输入 (PC13) */
    IO_INch_RfRx,              /* RF 433MHz 接收 (PB12) */

    IO_INch_Max                /* 输入通道总数 */
} io_in_ch_t;

typedef struct {
    GPIO_TypeDef *port;        /* GPIO 端口 */
    unsigned short pin;        /* GPIO 引脚 */
    unsigned char defaul;
} BoardGpioInConfig_t;

typedef struct {
    unsigned char IOin[IO_INch_Max];
    unsigned char IOout[IO_OUTch_Max];
} BoardBSPConfig_t;

/* ===========================================================================
 *                          板级初始化
 * =========================================================================== */
void Board_Init(void);
void Board_BSPfunc(void);

extern BoardBSPConfig_t s_boardBSP;
/* ===========================================================================
 *                          用户业务配置
 * =========================================================================== */

/* 1. 按键 */
#define keyNUM              IO_INch_Max
#define KEY_CH_test         0
#define KEY_CH_trig         0
#define KEYshortTIM         50
#define KEYlongTIM          1500

/* 2. LED */
#define LEDNUM              1
#define LED_FLASH_ONDELAY   500
#define LED_FLASH_OFFDELAY  500
#define LED_CH_W            0
#define LED_CH_R            1

#endif