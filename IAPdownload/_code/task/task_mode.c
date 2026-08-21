#include "config.h"

/* TASK1 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK_MODE_PRIO      osPriorityAboveNormal        /* 任务优先级 */
#define TASK_MODE_STK_SIZE  128                     /* 任务堆栈大小(4字节为单位) */
osThreadId_t            Task_mode_ID;               /* 任务ID */

void task_mode_creat(void){
    osThreadAttr_t attr = {0};
    attr.name       = "task_mode";
    attr.stack_size = TASK_MODE_STK_SIZE * 4;
    attr.priority   = TASK_MODE_PRIO;
    Task_mode_ID = osThreadNew(task_mode, NULL, &attr);
}

Str_tick Tick_str;
void tick_func(void);

void task_mode(void *pvParameters){
    uint32_t tick = osKernelGetTickCount();
    while(1){
        tick_func();

        key_func();
        LEDxfunc();
        
        Board_func();       // REG_out → GPIO输出
        
        modefunc_func();    // Prefunc清零 → mode置1 → Lastfunc驱动刷新
        
        tick += 1;
        osDelayUntil(tick);
    }
}



static void tick1ms(void){
}

static void tick1s(void){
    
}
void tick_init(void){
    memset(&Tick_str,0,sizeof(Str_tick));

}
void tick_func(void){
    tick1ms();
    if(Tick_str.u32systick < 0xffff0000){
        Tick_str.u32systick++;
    }
    Tick_str.u16_1mstick++;
    if(Tick_str.u16_1mstick >= 1000){
        Tick_str.u16_1mstick = 0;
        tick1s();
    }
}