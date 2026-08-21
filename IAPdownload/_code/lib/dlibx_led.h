/**
 * @file    dlibx_led.h
 * @brief   LED多通道驱动
 * @details 基于 dlibx 的 LedStruct 实现多通道LED管理
 *          支持常亮、关闭、闪烁三种状态
 *
 * 依赖：
 *   - dlibx.h（LedStruct、LED_statefunc）
 *   - boarddef.h（需提供 LEDNUM、LED_FLASH_ONDELAY、LED_FLASH_OFFDELAY、LED_CH_DS0 等宏）
 */

#ifndef __dlibx_led_h__
#define __dlibx_led_h__

#define DLIBX_LED_FLASH_500MS 500
#define DLIBX_LED_FLASH_630MS 630

void dlibx_led_init(void);
void dlibx_led_tick(void);
void dlibx_led_func(void);

void dlibx_led_ctrl(unsigned char sel,unsigned char state,unsigned short halfT);
#endif
