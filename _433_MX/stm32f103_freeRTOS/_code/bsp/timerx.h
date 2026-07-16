#ifndef __timerx_h__ 
#define __timerx_h__ 

#define PWM_LED_PERIOD  (1000)
typedef struct{
	unsigned char u8_1msflag;
	unsigned short int u16_1mstick;
    unsigned u32systick;
}Str_timer;

#define TIME100US_(x)        (x)
#define TIME1MS_(x)          (x)
#define TIME10MS_(x)         (x)
#define TIME100MS_(x)        (x)
#define TIME1S_(x)           (x)

extern Str_timer TIM_str;

void timerx_init(void);
void timerx_func(void); 

#endif 
