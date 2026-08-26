#ifndef __key_h__
#define __key_h__

void key_init(void);
void key_func(void);

unsigned int keyXvalread(unsigned char ch,unsigned char valnum);

/**
 * @brief   读取按键事件（短按），读后自动清除
 * @param   ch: IO_INch_KEY_UP/DOWN/LEFT/RIGHT
 * @return  1=有短按事件, 0=无
 * @note    配合10ms UI刷新周期使用，不会漏检
 */
unsigned char key_event_read(unsigned char ch);

/**
 * @brief   读取当前按键物理状态
 * @param   ch: IO_INch_KEY_UP/DOWN/LEFT/RIGHT
 * @return  1=按下, 0=释放
 */
unsigned char key_state_read(unsigned char ch);

#endif 
