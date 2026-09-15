/**
 ******************************************************************************
 * @file    system_stm32f4xx.c
 * @brief   CMSIS Cortex-M4 Device Peripheral Access Layer System Source File
 * @details 此文件提供系统初始化和时钟配置
 *          - STM32F407 时钟配置: HSE 8MHz -> PLL -> SYSCLK 168MHz
 *          - AHB = 168MHz, APB1 = 42MHz, APB2 = 84MHz
 ******************************************************************************
 */

#include "stm32f4xx.h"
#include "stm32f4xx_ll_rcc.h"

/* 系统时钟频率变量 */
uint32_t SystemCoreClock = 168000000U;
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8] = {0, 0, 0, 0, 1, 2, 3, 4};

/**
 * @brief  Setup the microcontroller system
 *         Initialize the FPU setting and vector table location
 * @param  None
 * @retval None
 */
void SystemInit(void)
{
    /* FPU settings */
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));  /* set CP10 and CP11 Full Access */
#endif

    /* Reset the RCC clock configuration to the default reset state */
    /* Set HSION bit */
    RCC->CR |= (uint32_t)0x00000001;

    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x24003010;

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Disable all interrupts */
    RCC->CIR = 0x00000000;

    /* Configure the Vector Table location */
#if defined(VECT_TAB_SRAM)
    SCB->VTOR = SRAM_BASE | 0x0;
#else
    SCB->VTOR = FLASH_BASE | 0x0;
#endif
}

/**
 * @brief  Update SystemCoreClock variable according to Clock Register Values
 * @note   此函数在时钟配置改变后调用，更新SystemCoreClock变量
 * @param  None
 * @retval None
 */
void SystemCoreClockUpdate(void)
{
    uint32_t tmp = 0, pllvco = 0, pllp = 2, pllsource = 0, pllm = 2;

    /* Get SYSCLK source */
    tmp = RCC->CFGR & RCC_CFGR_SWS;

    switch (tmp)
    {
    case 0x00:  /* HSI used as system clock source */
        SystemCoreClock = HSI_VALUE;
        break;
    case 0x04:  /* HSE used as system clock source */
        SystemCoreClock = HSE_VALUE;
        break;
    case 0x08:  /* PLL used as system clock source */
        /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N
           SYSCLK = PLL_VCO / PLL_P */
        pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
        pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;

        if (pllsource != 0)
        {
            /* HSE used as PLL clock source */
            pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
        }
        else
        {
            /* HSI used as PLL clock source */
            pllvco = (HSI_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
        }

        pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> 16) + 1) * 2;
        SystemCoreClock = pllvco / pllp;
        break;
    default:
        SystemCoreClock = HSI_VALUE;
        break;
    }

    /* Compute HCLK clock frequency */
    /* Get HCLK prescaler */
    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    /* HCLK clock frequency */
    SystemCoreClock >>= tmp;
}
