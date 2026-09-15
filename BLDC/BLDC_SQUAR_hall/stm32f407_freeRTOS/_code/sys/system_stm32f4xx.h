/**
 ******************************************************************************
 * @file    system_stm32f4xx.h
 * @brief   CMSIS Cortex-M4 Device System Header File
 ******************************************************************************
 */

#ifndef __SYSTEM_STM32F4XX_H
#define __SYSTEM_STM32F4XX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @brief CMSIS 系统时钟频率变量 */
extern uint32_t SystemCoreClock;

/**
 * @brief  Setup the microcontroller system.
 *         Initialize the FPU setting and vector table location.
 * @param  None
 * @retval None
 */
extern void SystemInit(void);

/**
 * @brief  Update SystemCoreClock variable according to Clock Register Values.
 * @param  None
 * @retval None
 */
extern void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_STM32F4XX_H */
