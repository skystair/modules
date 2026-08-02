#include "config.h"

keyStruct keystr[IO_INch_Max];
//定义按键读取
unsigned char keyRead_test(void){
    return Board_REG_in[IO_INch_KEY_USR];
}
unsigned char keyRead_wakeup(void){
    return Board_REG_in[IO_INch_KEY_WAKEUP];
}

void key_init(void){
    memset(&keystr,0,sizeof(keyStruct) *IO_INch_Max);

    for(unsigned char i = 0;i<IO_INch_Max;i++){
        keystr[i].Sdelay = KEYshortTIM;
        keystr[i].Ldelay = KEYlongTIM;
    }
    keystr[IO_INch_KEY_USR].pRfunc = &keyRead_test;
    keystr[IO_INch_KEY_WAKEUP].pRfunc = &keyRead_wakeup;
}

unsigned int keyXvalread(unsigned char ch,unsigned char valnum){
return  keyValread(&keystr[ch],valnum);
}
//func
void key_func(void){
    unsigned char i;
    for(i = 0;i< IO_INch_Max;i++){
        keystr[i].u16tick++;
        keystr[i].u16Rtick++;
        keyShortPressCHK(&keystr[i]);
    }
}