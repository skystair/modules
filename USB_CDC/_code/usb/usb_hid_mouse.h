#ifndef __USB_HID_MOUSE_H__
#define __USB_HID_MOUSE_H__

#include <stdint.h>

void USB_HID_Init(void);

/* 1ms周期调用，直接透传按键状态给操作系统 */
void USB_HID_Mouse_Func(unsigned char btn_left, unsigned char btn_right);

#endif
