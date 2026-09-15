/**
 ******************************************************************************
 * @file    BLDC_hall.c
 * @brief   BLDC 霍尔传感器读取与处理
 ******************************************************************************
 */

#include "config.h"

/* 全局变量 */
BLDC_Hall_t g_hall;

/* Hall 状态 -> 扇区映射表 (根据仿真测试验证)
 * sector 1~6 顺序循环，state 对应次序: 5, 1, 3, 2, 6, 4
 */
static const unsigned char hall_to_sector[8] = {
    0,  /* 000 - 无效 */
    2,  /* 001 - sector 2 */
    4,  /* 010 - sector 4 */
    3,  /* 011 - sector 3 */
    6,  /* 100 - sector 6 */
    1,  /* 101 - sector 1 */
    5,  /* 110 - sector 5 */
    0   /* 111 - 无效 */
};

/**
 * @brief Hall 传感器初始化
 */
void BLDC_Hall_Init(void)
{
    g_hall.state = 0;
    g_hall.state_prev = 0;
    g_hall.u = 0;
    g_hall.v = 0;
    g_hall.w = 0;
    g_hall.sector = 0;
}

/**
 * @brief 读取 Hall 传感器状态
 *        读取 PH10(U), PH11(V), PH12(W) 引脚电平
 */
void BLDC_Hall_Read(void)
{
    /* 保存上一次状态 */
    g_hall.state_prev = g_hall.state;

    /* 读取各相 Hall 状态 */
    g_hall.u = (unsigned char)LL_GPIO_IsInputPinSet(HALL_U_PORT, HALL_U_PIN);
    g_hall.v = (unsigned char)LL_GPIO_IsInputPinSet(HALL_V_PORT, HALL_V_PIN);
    g_hall.w = (unsigned char)LL_GPIO_IsInputPinSet(HALL_W_PORT, HALL_W_PIN);

    /* 组合为 3bit 状态值: bit2=U, bit1=V, bit0=W */
    g_hall.state = (g_hall.u << 2) | (g_hall.v << 1) | g_hall.w;

    /* 计算当前扇区 */
    g_hall.sector = BLDC_Hall_GetSector(g_hall.state);
}

/**
 * @brief 根据 Hall 状态获取扇区号
 * @param hall_state: 3bit Hall 状态值
 * @retval 扇区号 (1-6), 0 表示无效状态
 */
unsigned char BLDC_Hall_GetSector(unsigned char hall_state)
{
    if (hall_state > 7) return 0;
    return hall_to_sector[hall_state];
}
