#include "config.h"

keyStruct keystr[keyNUM];

unsigned char keyRead_test(void){
    return s_boardBSP.IOin[IO_INch_Key];
}

//GPIO 初始化已移至 Board_Init()
void keybspinit(void){
    /* GPIO 配置已由 Board_Init() 完成 */
}
void keyvalinit(void){
    memset(&keystr,0,sizeof(keyStruct) *keyNUM);
    for(unsigned char i = 0;i<keyNUM;i++){
        keystr[i].Sdelay = KEYshortTIM;
        keystr[i].Ldelay = KEYlongTIM;
    }
    keystr[KEY_CH_trig].pRfunc = &keyRead_test;
}

void key_init(void){
    keybspinit();
    keyvalinit();
}
//1mstick++
void key_tick(void){
    unsigned char i;
    for(i = 0;i< keyNUM;i++){
        keystr[i].u16tick++;
        keystr[i].u16Rtick++;
    }
}

unsigned int keyXvalread(unsigned char ch,unsigned char valnum){
return  keyValread(&keystr[ch],valnum);
}
//func
void key_func(void){
    keyShortPressCHK(&keystr[KEY_CH_test]);
}