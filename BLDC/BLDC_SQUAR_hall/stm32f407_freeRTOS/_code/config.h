#ifndef __config_h__
#define __config_h__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_pwr.h"

#include "dlibxConf.h"
#include "dlibx.h"

/* 用户模块头文件 */
#include "gpiox.h"
#include "timerx.h"
#include "bldc_func.h"
#include "bldc_hall.h"
#include "bldc_drive_6step.h"
#include "keyx.h"

/* 霍尔传感器输入引脚 */
#define HALL_U_PORT         GPIOH
#define HALL_U_PIN          LL_GPIO_PIN_10

#define HALL_V_PORT         GPIOH
#define HALL_V_PIN          LL_GPIO_PIN_11

#define HALL_W_PORT         GPIOH
#define HALL_W_PIN          LL_GPIO_PIN_12

/* 按键输入引脚 */
#define KEY0_PORT           GPIOE
#define KEY0_PIN            LL_GPIO_PIN_2

#define KEY1_PORT           GPIOE
#define KEY1_PIN            LL_GPIO_PIN_3

#define KEY2_PORT           GPIOE
#define KEY2_PIN            LL_GPIO_PIN_4

/* LED 输出引脚 */
#define LED0_PORT           GPIOE
#define LED0_PIN            LL_GPIO_PIN_0

#define LED1_PORT           GPIOE
#define LED1_PIN            LL_GPIO_PIN_1

/* BLDC 驱动引脚 - TIM1 */
/* 电源使能脚 (初始化后常开) */
#define PM1_CTRL_SD_PORT    GPIOF
#define PM1_CTRL_SD_PIN     LL_GPIO_PIN_10

/* 刹车信号 */
#define PM1_BRAKE_PORT      GPIOB
#define PM1_BRAKE_PIN       LL_GPIO_PIN_12

/* U 相上桥 PWM (TIM1_CH1) */
#define PM1_PWM_UH_PORT     GPIOA
#define PM1_PWM_UH_PIN      LL_GPIO_PIN_8

/* V 相上桥 PWM (TIM1_CH2) */
#define PM1_PWM_VH_PORT     GPIOA
#define PM1_PWM_VH_PIN      LL_GPIO_PIN_9

/* W 相上桥 PWM (TIM1_CH3) */
#define PM1_PWM_WH_PORT     GPIOA
#define PM1_PWM_WH_PIN      LL_GPIO_PIN_10

/* U 相下桥 PWM (TIM1_CH1N) */
#define PM1_PWM_UL_PORT     GPIOB
#define PM1_PWM_UL_PIN      LL_GPIO_PIN_13

/* V 相下桥 PWM (TIM1_CH2N) */
#define PM1_PWM_VL_PORT     GPIOB
#define PM1_PWM_VL_PIN      LL_GPIO_PIN_14

/* W 相下桥 PWM (TIM1_CH3N) */
#define PM1_PWM_WL_PORT     GPIOB
#define PM1_PWM_WL_PIN      LL_GPIO_PIN_15

#endif /* __config_h__ */

/*
PH10  HALL_U
PH11  HALL_V
PH12  HALL_W

PE2		KEY0
PE3		KEY1
PE4		KEY2

PE0		LED0
PE1		LED1

PF10	ADC3_IN8	PM1_CTRL_SD	Y	直流有/无刷驱动板1-停机控制信号
PB12	TIM1_BKIN	TIM1_BKIN	Y	直流有/无刷驱动板1-刹车信号

PA8	TIM1_CH1	PM1_PWM_UH	Y	直流有/无刷驱动板1-U相上桥PWM信号
PA9	TIM1_CH2	PM1_PWM_VH	Y	直流有/无刷驱动板1-V相上桥PWM信号
PA10	TIM1_CH3	PM1_PWM_WH	Y	直流有/无刷驱动板1-W相上桥PWM信号

PB13	TIM1_CH1N	PM1_PWM_UL	Y	直流有/无刷驱动板1-U相下桥PWM信号
PB14	TIM1_CH2N	PM1_PWM_VL	Y	直流有/无刷驱动板1-V相下桥PWM信号
PB15	TIM1_CH3N	PM1_PWM_WL	Y	直流有/无刷驱动板1-W相下桥PWM信号
*/