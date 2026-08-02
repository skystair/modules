#include "config.h"

keyStruct keystr[IO_INch_Max];
//定义按键读取
unsigned char keyRead_R(void){
    return Board_REG_in[IO_INch_KEY_R];
}
unsigned char keyRead_M(void){
    return Board_REG_in[IO_INch_KEY_M];
}
unsigned char keyRead_L(void){
    return Board_REG_in[IO_INch_KEY_L];
}
unsigned char keyRead_UP(void){
    return Board_REG_in[IO_INch_KEY_UP];
}
void key_init(void){
    memset(&keystr,0,sizeof(keyStruct) *IO_INch_Max);

    for(unsigned char i = 0;i<IO_INch_Max;i++){
        keystr[i].Sdelay = KEYshortTIM;
        keystr[i].Ldelay = KEYlongTIM;
    }
    keystr[IO_INch_KEY_R].pRfunc = &keyRead_R;
    keystr[IO_INch_KEY_M].pRfunc = &keyRead_M;
	keystr[IO_INch_KEY_L].pRfunc = &keyRead_L;
	keystr[IO_INch_KEY_UP].pRfunc = &keyRead_UP;
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