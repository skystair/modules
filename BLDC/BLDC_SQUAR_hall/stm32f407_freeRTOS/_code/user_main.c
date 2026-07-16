#include "main.h"

#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

//os------------------------------
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "freertos_evr.h"
#include "task.h"
#include "timers.h"

//
#include "task_drive.h"

void SystemClock_Config(void);

#if 1
unsigned int systemtick;
//systick**************************************************************
#if defined(SysTick)
#undef SysTick_Handler

/* CMSIS SysTick interrupt handler prototype */
extern void SysTick_Handler     (void);
/* FreeRTOS tick timer interrupt handler prototype */
extern void xPortSysTickHandler (void);

/**
 * @brief  SysTick中断处理函数
 * @details 清除溢出标志并调用FreeRTOS节拍处理器
 *          同时递增系统节拍计数器
 */
void SysTick_Handler (void) {
#if (configUSE_TICKLESS_IDLE == 0)
  /* Clear overflow flag */
  SysTick->CTRL;
#endif

  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    /* Call tick handler */
    xPortSysTickHandler();
  }
  systemtick++;
  //usr tick
//  key_tick();
}
#endif /* SysTick */
#endif

#if configGENERATE_RUN_TIME_STATS == 1
/******************runtim cal*****************************/
char runtimeinfo[200];
char* info = runtimeinfo;
/**
 * @brief  获取任务运行时统计信息
 * @details 将各任务的CPU使用率等信息存储到runtimeinfo数组中
 */
void taskinfo(void){
    vTaskGetRunTimeStats(info);             /* ?????????? */
}
uint32_t FreeRTOSRunTimeTicks;          /* FreeRTOS???????????? */
/**
 * @brief  配置运行时统计定时器
 * @details 初始化用于计算任务运行时间的定时器并启用中断
 */
void ConfigureTimeForRunTimeStats(void){//tim init and enable IT
    uint16_t                  u16ArrValue;
    uint16_t                  u16CntValue;
    stc_bt_mode0_cfg_t     stcBtBaseCfg;
    
    DDL_ZERO_STRUCT(stcBtBaseCfg);
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, TRUE); //Base Timer外设时钟使能
    
    stcBtBaseCfg.enWorkMode = BtWorkMode0;                  //定时器模式
    stcBtBaseCfg.enCT       = BtTimer;                      //定时器功能，计数时钟为内部PCLK
    stcBtBaseCfg.enPRS      = BtPCLKDiv16;                 //PCLK/256
    stcBtBaseCfg.enCntMode  = Bt16bitArrMode;               //自动重载16位计数器/定时器
    stcBtBaseCfg.bEnTog     = FALSE;
    stcBtBaseCfg.bEnGate    = FALSE;
    stcBtBaseCfg.enGateP    = BtGatePositive;
    Bt_Mode0_Init(TIM1, &stcBtBaseCfg);                     //TIM1 的模式0功能初始化
    
    u16ArrValue = 0x10000 - 3000;
    Bt_M0_ARRSet(TIM1, u16ArrValue);                        //设置重载值(ARR = 0x10000 - 周期)
    
    u16CntValue = 0x10000 - 3000;
    Bt_M0_Cnt16Set(TIM1, u16CntValue);                      //设置计数初值
    
    Bt_ClearIntFlag(TIM1,BtUevIrq);                         //清中断标志   
    Bt_Mode0_EnableIrq(TIM1);                               //使能TIM1中断(模式0时只有一个中断)
    EnableNvic(TIM1_IRQn, IrqLevel3, TRUE);                 //TIM1中断使能
    Bt_M0_Run(TIM1);
    FreeRTOSRunTimeTicks = 0;           /* ?????????0 */
}
void TIM1_IRQHandler(void){
    if(TRUE == Bt_GetIntFlag(TIM1, BtUevIrq)){
        Bt_ClearIntFlag(TIM1,BtUevIrq); //中断标志清零
        FreeRTOSRunTimeTicks++;
    }
}
#endif

int main(void){
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  
  
  task_drive_creat();
  #if configGENERATE_RUN_TIME_STATS == 1
    ConfigureTimeForRunTimeStats();
    #endif
  vTaskStartScheduler();
}
