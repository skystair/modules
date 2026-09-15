/**
 * @file    keyx.c
 * @brief   按键驱动 - 基于 dlibx keyShortPressCHK
 * @details KEY0/1/2 连接 PE2/3/4，下拉输入，按下为高
 *          keyx_tick1ms() 需在 SysTick 中断每 1ms 调用一次
 *          keyx_func()   在主循环中调用
 */

#include "config.h"

/* 按键结构体数组 (Pressing / flag / keep / u16tick 由 keyShortPressCHK 管理) */
keyStruct g_keys[KEY_NUM] = {
    /* Pressing, flag, keep, u16tick, u16Rtick, Sdelay, Ldelay */
    { 0, 0, 0, 0, 0, KEY_SDELAY, 0 },   /* KEY0 */
    { 0, 0, 0, 0, 0, KEY_SDELAY, 0 },   /* KEY1 */
    { 0, 0, 0, 0, 0, KEY_SDELAY, 0 },   /* KEY2 */
};

/* 短按触发标志 (边沿，读后自动清零) */
unsigned char g_key0_pressed = 0;
unsigned char g_key1_pressed = 0;
unsigned char g_key2_pressed = 0;

/**
 * @brief 按键初始化 (GPIO 已在 gpiox_init 中配置)
 */
void keyx_init(void)
{
    unsigned char i;
    for (i = 0; i < KEY_NUM; i++) {
        g_keys[i].Pressing  = 0;
        g_keys[i].flag      = 0;
        g_keys[i].keep      = 0;
        g_keys[i].u16tick   = 0;
        g_keys[i].u16Rtick  = 0;
    }
    g_key0_pressed = 0;
    g_key1_pressed = 0;
    g_key2_pressed = 0;
}

/**
 * @brief 1ms tick 累加 - 在 SysTick 中断中调用
 * @details 按键按下时 u16tick++，松开时 u16Rtick++ (由 keyShortPressCHK 管理)
 */
void keyx_tick1ms(void)
{
    unsigned char i;
    for (i = 0; i < KEY_NUM; i++) {
        if (g_keys[i].Pressing) {
            if (g_keys[i].u16tick < 0xFF00) {
                g_keys[i].u16tick++;
            }
        } else {
            if (g_keys[i].u16Rtick < 0xFF00) {
                g_keys[i].u16Rtick++;
            }
        }
    }
}

/**
 * @brief 按键主循环处理 - 在主循环中调用
 * @details 读 GPIO → 更新 Pressing → keyShortPressCHK → 更新 pressed 标志
 */
void keyx_func(void)
{
    /* 读取 GPIO 电平，更新 Pressing 状态 (下拉输入，按下为高) */
    g_keys[0].Pressing = LL_GPIO_IsInputPinSet(KEY0_PORT, KEY0_PIN) ? 1 : 0;
    g_keys[1].Pressing = LL_GPIO_IsInputPinSet(KEY1_PORT, KEY1_PIN) ? 1 : 0;
    g_keys[2].Pressing = LL_GPIO_IsInputPinSet(KEY2_PORT, KEY2_PIN) ? 1 : 0;

    /* dlibx 短按检测 */
    keyShortPressCHK(&g_keys[0]);
    keyShortPressCHK(&g_keys[1]);
    keyShortPressCHK(&g_keys[2]);

    /* 更新短按标志 (边沿触发) */
    if (g_keys[0].flag == KEY_FALG_SHORT) {
        g_key0_pressed = 1;
    }
    if (g_keys[1].flag == KEY_FALG_SHORT) {
        g_key1_pressed = 1;
    }
    if (g_keys[2].flag == KEY_FALG_SHORT) {
        g_key2_pressed = 1;
    }
}
