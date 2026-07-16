/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define key0_Pin GPIO_PIN_2
#define key0_GPIO_Port GPIOE
#define key1_Pin GPIO_PIN_3
#define key1_GPIO_Port GPIOE
#define key2_Pin GPIO_PIN_4
#define key2_GPIO_Port GPIOE
#define BLDC_CTL_PWE_Pin GPIO_PIN_10
#define BLDC_CTL_PWE_GPIO_Port GPIOF
#define BLDC_HALL_U_Pin GPIO_PIN_10
#define BLDC_HALL_U_GPIO_Port GPIOH
#define BLDC_HALL_V_Pin GPIO_PIN_11
#define BLDC_HALL_V_GPIO_Port GPIOH
#define BLDC_HALL_W_Pin GPIO_PIN_12
#define BLDC_HALL_W_GPIO_Port GPIOH
#define BLDC_LB_U_Pin GPIO_PIN_13
#define BLDC_LB_U_GPIO_Port GPIOB
#define BLDC_LB_V_Pin GPIO_PIN_14
#define BLDC_LB_V_GPIO_Port GPIOB
#define BLDC_LB_W_Pin GPIO_PIN_15
#define BLDC_LB_W_GPIO_Port GPIOB
#define BLDC_HB_U_Pin GPIO_PIN_8
#define BLDC_HB_U_GPIO_Port GPIOA
#define BLDC_HB_V_Pin GPIO_PIN_9
#define BLDC_HB_V_GPIO_Port GPIOA
#define BLDC_HB_W_Pin GPIO_PIN_10
#define BLDC_HB_W_GPIO_Port GPIOA
#define LOG_TX_Pin GPIO_PIN_6
#define LOG_TX_GPIO_Port GPIOB
#define LOG_RX_Pin GPIO_PIN_7
#define LOG_RX_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_0
#define LED_G_GPIO_Port GPIOE
#define LED_R_Pin GPIO_PIN_1
#define LED_R_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
