/**
 ******************************************************************************
 * @file    stm32f4xx_conf.h
 * @brief   STM32F4xx LL库配置文件
 * @details 此文件用于配置STM32F4xx LL库的外设模块
 ******************************************************************************
 */

#ifndef __STM32F4xx_CONF_H
#define __STM32F4xx_CONF_H

/* Includes ------------------------------------------------------------------*/
/* LL库模块使能 - 根据需要取消注释 */

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_adc.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_cortex.h"

/* 断言配置 */
/* #define USE_FULL_ASSERT    1U */

#ifdef USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t* file, uint32_t line);
#else
#define assert_param(expr) ((void)0U)
#endif

#endif /* __STM32F4xx_CONF_H */
