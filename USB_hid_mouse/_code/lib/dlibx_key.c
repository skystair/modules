/**
 * @file    dlibx_key.c
 * @brief   按键多通道驱动实现
 *
 * 依赖：
 *   - dlibx.h（keyStruct、keyShortPressCHK、keyLongPressCHK、memset）
 *   - boardIO.h（需提供 s_boardBSP、IO_INch_Key、keyNUM 等宏）
 *   - 使用前需在 boardIO.h 中定义按键通道相关宏
 */

#include "dlibxConf.h"
//#include "boardIO.h"
#include "dlibx_key.h"
#include <string.h>

keyStruct dlibx_keystr[keyNUM];

static unsigned char dlibx_keyRead_test(void){
    unsigned char ret = 0;
//    ret = s_boardBSP.IOin[IO_INch_Key];
    return ret;
}

static void dlibx_keybspinit(void){
    /* GPIO 配置已由 Board_Init() 完成 */
}
static void dlibx_keyvalinit(void){
    memset(&dlibx_keystr,0,sizeof(keyStruct) *keyNUM);
    for(unsigned char i = 0;i<keyNUM;i++){
        dlibx_keystr[i].Sdelay = KEYshortTIM;
        dlibx_keystr[i].Ldelay = KEYlongTIM;
    }

    dlibx_keystr[KEY_CH_trig].pRfunc = &dlibx_keyRead_test;
    //other key channel init...
}

void dlibx_key_init(void){
    dlibx_keybspinit();
    dlibx_keyvalinit();
}

void dlibx_key_tick(void){
    unsigned char i;
    for(i = 0;i< keyNUM;i++){
        dlibx_keystr[i].u16tick++;
        dlibx_keystr[i].u16Rtick++;
    }
}

unsigned int dlibx_keyXvalread(unsigned char ch,unsigned char valnum){
    return keyValread(&dlibx_keystr[ch],valnum);
}

void dlibx_key_func(void){
    keyShortPressCHK(&dlibx_keystr[KEY_CH_test]);
}
