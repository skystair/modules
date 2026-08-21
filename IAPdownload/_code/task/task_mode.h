#ifndef __task_mode_h__ 
#define __task_mode_h__ 

typedef struct{
	unsigned char u8_1msflag;
	unsigned short int u16_1mstick;
    unsigned u32systick;
}Str_tick;

#define TIME100US_(x)        (x)
#define TIME1MS_(x)          (x)
#define TIME10MS_(x)         (x)
#define TIME100MS_(x)        (x)
#define TIME1S_(x)           (x)

void tick_init(void);
void task_mode_creat(void);
void task_mode(void *pvParameters);             /* 任务函数 */

#endif 
