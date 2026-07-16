#include "config.h"
#include "RF_433_MX_test_tx.h"
#include "RF_433_MX_uart.h"

/* TASK1 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK_MODE_PRIO      8                   /* 任务优先级 */
#define TASK_MODE_STK_SIZE  128                 /* 任务堆栈大小 */
TaskHandle_t            Task_mode_Handler;  /* 任务句柄 */

void task_mode_creat(void){
//    RF_433_MX_test_tx_init();  /* 已注释：旧TX测试初始化 */
    xTaskCreate((TaskFunction_t )task_mode,                 /* 任务函数 */
                (const char*    )"task_mode",               /* 任务名称 */
                (uint16_t       )TASK_MODE_STK_SIZE,        /* 任务堆栈大小 */
                (void*          )NULL,                  /* 传入给任务函数的参数 */
                (UBaseType_t    )TASK_MODE_PRIO,            /* 任务优先级 */
                (TaskHandle_t*  )&Task_mode_Handler);   /* 任务句柄 */
}

void task_mode(void *pvParameters){
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS( 1 );

    unsigned short uartWakeupCnt = 0;  /* UART唤醒延迟计数 */

    while(1){
        /* 1ms 周期任务 */
        key_tick();
        LEDxtick();
        RF_433_MX_tick1ms();   /* 已注释：旧TX tick（UART方案不需要） */

        /* 逻辑处理 */
        Board_BSPfunc();
        timerx_func();
        key_func();
        LEDxfunc();
//        /* 上电100ms后使能EXTI10唤醒中断（等AGC稳定） */
//        if (uartWakeupCnt < 100) {
//            uartWakeupCnt++;
//            if (uartWakeupCnt >= 100) {
//                RF_433_MX_uart_enableWakeup();
//            }
//        }
//        /* 按键短按 → 发送测试数据 */
//        if(keyXvalread(KEY_CH_test, KEY_VAL_FLAG)){
//            RF_433_MX_uart_TXtrig();
//        }
//        RF_433_MX_uart_func();
        
        xTaskDelayUntil( &xLastWakeTime, xFrequency);
    }
}
