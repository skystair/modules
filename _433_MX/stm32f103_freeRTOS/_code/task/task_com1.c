#include "config.h"
#include "RF_433_MX_uart.h"  /* 替代 RF_433_MX_test_rx.h */

/* TASK2 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK_com1_PRIO      8                   /* 任务优先级 */
#define TASK_com1_STK_SIZE  256                 /* 任务堆栈大小 */
TaskHandle_t            Task_com1_Handler;  /* 任务句柄 */

void task_com1_creat(void){
//    RF_433_MX_test_rx_init();  /* 已注释：旧RX测试初始化 */
    xTaskCreate((TaskFunction_t )task_com1,                 /* 任务函数 */
                (const char*    )"task_com1",               /* 任务名称 */
                (uint16_t       )TASK_com1_STK_SIZE,        /* 任务堆栈大小 */
                (void*          )NULL,                  /* 传入给任务函数的参数 */
                (UBaseType_t    )TASK_com1_PRIO,            /* 任务优先级 */
                (TaskHandle_t*  )&Task_com1_Handler);   /* 任务句柄 */
}

void task_com1(void *pvParameters){
    while(1){

        vTaskDelay(5);  /* 5ms 轮询间隔 */
    }
}