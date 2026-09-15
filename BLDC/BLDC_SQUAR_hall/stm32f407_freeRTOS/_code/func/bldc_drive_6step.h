/**
 ******************************************************************************
 * @file    bldc_drive_6step.h
 * @brief   BLDC 6步换向驱动
 ******************************************************************************
 */

#ifndef __bldc_drive_6step_h__
#define __bldc_drive_6step_h__

/* 6步换向状态定义 */
/* 上桥臂: 1=PWM, 0=关断 */
/* 下桥臂: 1=常开, 0=关断 */
typedef struct {
    unsigned char uh;   /* U相上桥 */
    unsigned char ul;   /* U相下桥 */
    unsigned char vh;   /* V相上桥 */
    unsigned char vl;   /* V相下桥 */
    unsigned char wh;   /* W相上桥 */
    unsigned char wl;   /* W相下桥 */
} Drive_Step_t;

/* 6步换向表 (根据仿真测试: sector 1~6, state 5,1,3,2,6,4) */
/* 步序: 1=U+V-, 2=U+W-, 3=V+W-, 4=V+U-, 5=W+U-, 6=W+V- */
extern const Drive_Step_t drive_step_table[7];

/* 全局变量 */
extern unsigned short g_pwm_duty;   /* PWM 占空比 */
extern unsigned char g_drive_enable; /* 驱动使能标志 */

/* 函数声明 */
void BLDC_Drive_Init(void);
void BLDC_Drive_Enable(void);
void BLDC_Drive_Disable(void);
void BLDC_Drive_SetPWM(unsigned short duty);
void BLDC_Drive_Commutation(unsigned char sector);
void BLDC_Drive_Stop(void);

#endif /* __bldc_drive_6step_h__ */
