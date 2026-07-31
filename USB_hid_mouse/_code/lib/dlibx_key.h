/**
 * @file    dlibx_key.h
 * @brief   按键多通道驱动
 * @details 基于 dlibx 的 keyStruct 实现多通道按键管理
 *          支持短按、长按检测
 *
 * 依赖：
 *   - dlibx.h（keyStruct、keyShortPressCHK、keyLongPressCHK）
 *   - boardIO.h（需提供 keyNUM、KEYshortTIM、KEYlongTIM、KEY_CH_trig 等宏）
 */

#ifndef __dlibx_key_h__
#define __dlibx_key_h__


void dlibx_key_init(void);
void dlibx_key_tick(void);
void dlibx_key_func(void);

unsigned int dlibx_keyXvalread(unsigned char ch,unsigned char valnum);
#endif
