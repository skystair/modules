#include "main.h"

//os------------------------------
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "freertos_evr.h"
#include "task.h"
#include "timers.h"

#if 1
void task_drive_creat(void); 
void task_drive_func(void *pvParameters); 

/* TASK1 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK_MODE_PRIO      8                   /* 任务优先级 */
#define TASK_MODE_STK_SIZE  256                /* 任务堆栈大小 */
TaskHandle_t                Task_mode_Handler;  /* 任务句柄 */

void task_drive_creat(void){
    /* 创建任务1 */
    xTaskCreate((TaskFunction_t )task_drive_func,       /* 任务函数 */
                (const char*    )"task_drive",          /* 任务名称 */
                (uint16_t       )TASK_MODE_STK_SIZE,    /* 任务堆栈大小 */
                (void*          )NULL,                  /* 传入给任务函数的参数 */
                (UBaseType_t    )TASK_MODE_PRIO,        /* 任务优先级 */
                (TaskHandle_t*  )&Task_mode_Handler);   /* 任务句柄 */
}
#endif

unsigned char testPINflag = 0;
void comman1mstick(void){
   
}
unsigned int testi = 1;
static void delayxx(unsigned int xxx){
    int i,j;
    for(j = 0;j<xxx;j++)
    for(i = 0;i<0x0000ffff;i++){
        __NOP();
    }
}
#define EN_USR_TEST_PIN
void task_drive_func(void *pvParameters){
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS( 1000 );
    
    while(1){
        #ifdef TASK_INFO_CHK
        taskinfo();
        #endif
        
        comman1mstick();
        //bsp - 始终运行
        
        delayxx(testi);
        #ifdef EN_USR_TEST_PIN
        testPINflag ^= 0xff;
        if(testPINflag){
            HAL_GPIO_WritePin(GPIOE, LED_G_Pin|LED_R_Pin, GPIO_PIN_RESET);
        }else{
            HAL_GPIO_WritePin(GPIOE, LED_G_Pin|LED_R_Pin, GPIO_PIN_SET);
        }
        #endif
        xTaskDelayUntil( &xLastWakeTime, xFrequency);
    }
}

//vTaskDelay(1);