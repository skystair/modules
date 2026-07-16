#ifndef __RF_433_MX_h__
#define __RF_433_MX_h__

#include "RF_433_MX_TX.h"
#include "RF_433_MX_RX.h"

/*
 * 433MHz 无线模块统一接口
 * 发射器: MX-FS-03V — 软件状态机，需在主循环调用 tick1ms
 * 接收器: MX-05V    — EXTI+TIM3 中断驱动，无需轮询
 *
 * 引脚分配（需在平台层定义）:
 *   TX: 推挽输出
 *   RX: 浮空输入，EXTI上升沿触发
 */

/* 平台初始化（GPIO + EXTI + TIM），需最先调用 */
void RF_433_MX_PlatformInit(void);

/* 模块初始化（状态机初始化） */
void RF_433_MX_init(void);

/* TX 定时器 tick（由定时器中断或主循环每 1ms 调用，仅影响 TX） */
void RF_433_MX_tick1ms(void);

#endif
