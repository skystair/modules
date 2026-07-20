#ifndef __dlibxconf_h__
#define __dlibxconf_h__

#include "dlibx.h"

#define keyNUM              1
#define KEY_CH_test         0
#define KEY_CH_trig         0
#define KEYshortTIM         50
#define KEYlongTIM          1500

extern keyStruct dlibx_keystr[keyNUM];
/* 2. LED */
#define LEDNUM              1
#define LED_FLASH_ONDELAY   500
#define LED_FLASH_OFFDELAY  500
#define LED_CH_W            0
#define LED_CH_R            1

extern LedStruct dlibx_ledstr[LEDNUM];



extern STRUart uart_exp1;                            /* 默认 UART 实例（保留兼容的单实例名） */
#endif
