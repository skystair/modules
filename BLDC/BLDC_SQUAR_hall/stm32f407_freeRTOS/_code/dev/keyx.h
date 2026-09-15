#ifndef __keyx_h__
#define __keyx_h__

#define KEY_NUM         3
#define KEY_SDELAY      20      /* 20ms 短按消抖 */

extern keyStruct g_keys[KEY_NUM];

extern unsigned char g_key0_pressed;
extern unsigned char g_key1_pressed;
extern unsigned char g_key2_pressed;

void keyx_init(void);
void keyx_tick1ms(void);
void keyx_func(void);

#endif /* __keyx_h__ */
