#include "config.h"

/* 速度参数 (PWM duty 0~1000) */
#define SPEED_MIN       100     /* 最低启动占空比 10% */
#define SPEED_MAX       1000    /* 最大占空比 100% */
#define SPEED_STEP      50      /* 每次加/减步进 5% */

/* 电机状态 */
unsigned char  g_motor_switch = 0;   /* 0=关闭, 1=运行 */
unsigned short g_motor_duty  = 0;    /* 当前 PWM 占空比 */

/**
 * @brief BLDC 功能初始化
 */
void bldc_func_init(void)
{
    BLDC_Hall_Init();
    BLDC_Drive_Init();
    keyx_init();

    g_motor_switch = 0;
    g_motor_duty   = SPEED_MIN;
}

/**
 * @brief BLDC 1ms 周期处理
 */
void bldc_func_tick1ms(void)
{
}

/**
 * @brief BLDC 主循环处理
 * @details
 *   KEY0 - 开/关电机 (开启时从最低速启动)
 *   KEY1 - 减速 (到最小后无效)
 *   KEY2 - 加速 (到最大后无效)
 */
void bldc_func_func(void)
{
    keyx_func();

    /* KEY0: 开/关电机 */
    if (g_key0_pressed) {
        g_key0_pressed = 0;
        if (g_motor_switch == 0) {
            g_motor_switch = 1;
            g_motor_duty   = SPEED_MIN;             /* 从低速启动 */
            BLDC_Drive_SetPWM(g_motor_duty);
        } else {
            g_motor_switch = 0;
            BLDC_Drive_Stop();
        }
    }

    if (g_motor_switch == 0) return;

    /* KEY1: 减速 */
    if (g_key1_pressed) {
        g_key1_pressed = 0;
        if (g_motor_duty > SPEED_MIN + SPEED_STEP) {
            g_motor_duty -= SPEED_STEP;
        } else {
            g_motor_duty = SPEED_MIN;
        }
        BLDC_Drive_SetPWM(g_motor_duty);
    }

    /* KEY2: 加速 */
    if (g_key2_pressed) {
        g_key2_pressed = 0;
        if (g_motor_duty < SPEED_MAX - SPEED_STEP) {
            g_motor_duty += SPEED_STEP;
        } else {
            g_motor_duty = SPEED_MAX;
        }
        BLDC_Drive_SetPWM(g_motor_duty);
    }

    /* Hall 读取 + 6步换向 */
    BLDC_Hall_Read();
    if (g_hall.sector > 0 && g_hall.sector <= 6) {
        BLDC_Drive_Commutation(g_hall.sector);
    }
} 
