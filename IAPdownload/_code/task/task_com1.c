#include "config.h"

/* TASK2 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK_com1_PRIO      osPriorityNormal        /* 任务优先级 */
#define TASK_com1_STK_SIZE  128                     /* 任务堆栈大小(4字节为单位) */
osThreadId_t            Task_com1_ID;               /* 任务ID */

void task_com1_creat(void){
    osThreadAttr_t attr = {0};
    attr.name       = "task_com1";
    attr.stack_size = TASK_com1_STK_SIZE * 4;
    attr.priority   = TASK_com1_PRIO;
    Task_com1_ID = osThreadNew(task_com1, NULL, &attr);
}

void task_com1(void *pvParameters){
    unsigned char flag = 0;
    
    while(1){
        /* UI刷新 - 200ms节流，内部处理状态变化和周期刷新 */
        ui_refresh();
        
//        LEDxCtrl(IO_OUTch_LED_DS0,(flag!=0),0);
        osDelay(10);  /* 10ms周期 */
        flag ^=1;
    }
}