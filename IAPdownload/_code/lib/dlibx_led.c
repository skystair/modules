/**
 * @file    dlibx_led.c
 * @brief   LED多通道驱动实现
 *
 * 依赖：
 *   - dlibx.h（LedStruct、LED_statefunc、memset）
 *   - boardIO.h（需提供 s_boardBSP、IO_OUTch_Led、LEDNUM 等宏）
 *   - 使用前需在 boardIO.h 中定义 LED 通道相关宏
 */
/* 2. LED */
#define LEDNUM              1
#define LED_FLASH_ONDELAY   500
#define LED_FLASH_OFFDELAY  500
#define LED_CH_DS0          0
#define LED_CH_DS1          1

#include "dlibxConf.h"
//#include "boardIO.h"
#include "dlibx_led.h"
#include <string.h>

LedStruct dlibx_ledstr[LEDNUM];

static void dlibx_led_testCtrl(unsigned char flag){
    // if(flag){
    //     s_boardBSP.IOout[IO_OUTch_Led] = 1;
    // }else{
    //     s_boardBSP.IOout[IO_OUTch_Led] = 0;
    // }
}

void dlibx_led_init(void){
    memset(&dlibx_ledstr,0,LEDNUM*sizeof(LedStruct));
    for(unsigned char i = 0; i< LEDNUM;i++){
        dlibx_ledstr[i].Flashondelay = LED_FLASH_ONDELAY;
        dlibx_ledstr[i].Flashoffdelay = LED_FLASH_OFFDELAY;
    }
    dlibx_ledstr[LED_CH_DS0].pfunc = &dlibx_led_testCtrl;
    //other led channel init...
}

void dlibx_led_tick(void){
    for(unsigned char i = 0; i< LEDNUM;i++){
        dlibx_ledstr[i].Flashtick++;
    }
}

void dlibx_led_func(void){
    for(unsigned char i = 0; i< LEDNUM;i++){
        LED_statefunc(&dlibx_ledstr[i]);
    }
}

void dlibx_led_ctrl(unsigned char sel,unsigned char state,unsigned short halfT){
    if(sel >= LEDNUM) return;
    dlibx_ledstr[sel].state = state;
    if(state == LED_STATE_FLASH){
        dlibx_ledstr[sel].Flashondelay = halfT;
        dlibx_ledstr[sel].Flashoffdelay = halfT;
    }
}
