
/**
 ******************************************************************************
 * @file    main.c
 * @brief   BLDC 电机控制主程序
 ******************************************************************************
 */

#include "config.h"

/* 系统时钟配置函数声明 */
static void SystemClock_Config(void);

/**
 * @brief  主函数入口
 * @retval int
 */
int main(void)
{
    /* 配置系统时钟: HSE 8MHz -> PLL -> SYSCLK 168MHz */
    SystemClock_Config();

    /* 初始化 SysTick */
    LL_Init1msTick(SystemCoreClock);
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;  /* 使能 SysTick 中断 */

    /* BSP 初始化 */
    gpiox_init();
    timerx_init();

    /* BLDC 功能初始化 */
    bldc_func_init();

    /* 启动调度器 (FreeRTOS) */
    /* vTaskStartScheduler(); */

    /* 主循环 */
    while (1)
    {
        /* 主循环处理 */
        gpiox_func();
        
        bldc_func_func();
    }
}

/**
 * @brief  系统时钟配置
 *         HSE = 8MHz
 *         PLL_M = 8, PLL_N = 336, PLL_P = 2
 *         SYSCLK = 168MHz
 *         AHB = 168MHz, APB1 = 42MHz, APB2 = 84MHz
 * @retval None
 */
static void SystemClock_Config(void)
{
    /* 使能电源时钟 */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    /* 设置电压调节器输出电压 */
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

    /* 使能 HSE */
    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() != 1U)
    {
        /* 等待 HSE 就绪 */
    }

    /* 配置 PLL */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_8, 336U, LL_RCC_PLLP_DIV_2);

    /* 使能 PLL */
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1U)
    {
        /* 等待 PLL 就绪 */
    }

    /* 配置 Flash 延迟 */
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
    if (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_5)
    {
        /* 错误处理 */
        while (1)
        {
        }
    }

    /* 配置 AHB/APBx 分频 */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);

    /* 配置 SysTick */
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
    {
        /* 等待 PLL 作为系统时钟源 */
    }

    /* 更新系统时钟变量 */
    SystemCoreClock = 168000000U;
}

/**
 * @brief  错误处理函数
 * @retval None
 */
void Error_Handler(void)
{
    while (1)
    {
    }
} 
