#include "config.h"

Str_timer TIM_str;


/**
 * @brief  基准定时器初始化
 * @param  None
 * @retval None
 */
void timerx_init(void){
    memset(&TIM_str,0,sizeof(Str_timer));
//    // Basetimer  Gtim0 0.1ms
//    stc_gtim_init_t stcGtimInit = {0};
//    
//    GTIM_StcInit(&stcGtimInit);                                     /* 结构体变量初始值初始化 */
//    stcGtimInit.u32Mode            = GTIM_MD_PCLK;                  /* 工作模式: 定时器模式，计数时钟源来自PCLK */
//    stcGtimInit.u32OneShotEn       = GTIM_CONTINUOUS_COUNTER;       /* 连续计数模式 */
//    stcGtimInit.u32Prescaler       = GTIM_COUNTER_CLK_DIV16;         /* 对计数时钟进行预除频 */
//    stcGtimInit.u32ToggleEn        = GTIM_TOGGLE_DISABLE;           /* TOG输出禁止 */
//    stcGtimInit.u32AutoReloadValue = PWM_LED_PERIOD - 1;                      /* 自动重载寄存ARR赋值,计数周期为PRS*(ARR+1)*TPCLK */
//    GTIM_Init(BASE_TIMER, &stcGtimInit);                            /* GTIM0初始化 */
//    GTIM_CompareCaptureAllDisable(BASE_TIMER); 						/* 禁止所有通道比较捕获功能 */
//    GTIM_IntFlagClear(BASE_TIMER, GTIM_FLAG_UI);         			/* 清除溢出中断标志位 */
//    GTIM_IntEnable(BASE_TIMER, GTIM_INT_UI);              			/* 允许GTIM0溢出中断    */
//    EnableNvic(CTIM0_IRQn, IrqPriorityLevel3, TRUE); 				/* NVIC 中断使能 */
//    GTIM_Enable(BASE_TIMER);
//    
//    GTIM_CompareCaptureModeSet(BASE_TIMER, GTIM_COMPARE_CAPTURE_CH0, GTIM_COMPARE_CAPTURE_PWM_INVERTED); 			/*!< PWM反向输出(CNT < CCR输出高电平) */
//    GTIM_CompareCaptureRegSet(BASE_TIMER, GTIM_COMPARE_CAPTURE_CH0, 0);                            					/* 比较值 20000 */
//    GTIM_Enable(BASE_TIMER);

}

void timerx_tick1s(void){
    
}

void timerx_func(void){
    if(TIM_str.u32systick < 0xffff0000){
        TIM_str.u32systick++;
    }
    TIM_str.u16_1mstick++;
    if(TIM_str.u16_1mstick >= 1000){
        TIM_str.u16_1mstick = 0;
        timerx_tick1s();
    }
}

