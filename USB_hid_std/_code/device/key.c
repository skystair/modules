#include "config.h"

keyStruct keystr[IO_INch_Max];
//瀹氫箟鎸夐敭璇诲彇
unsigned char keyRead_test(void){
    return Board_REG_in[IO_INch_KEY_USR];
}

void key_init(void){
    memset(&keystr,0,sizeof(keyStruct) *IO_INch_Max);
    
    for(unsigned char i = 0;i<IO_INch_Max;i++){
        keystr[i].Sdelay = KEYshortTIM;
        keystr[i].Ldelay = KEYlongTIM;
    }
    keystr[IO_INch_KEY_USR].pRfunc = &keyRead_test;
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