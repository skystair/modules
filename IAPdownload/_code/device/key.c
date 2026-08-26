#include "config.h"

keyStruct keystr[IO_INch_Max];

//按键读取函数
unsigned char keyRead_key_up(void){
    return Board_REG_in[IO_INch_KEY_UP];
}
unsigned char keyRead_key_right(void){
    return Board_REG_in[IO_INch_KEY_RIGHT];
}
unsigned char keyRead_key_down(void){
    return Board_REG_in[IO_INch_KEY_DOWN];
}
unsigned char keyRead_key_left(void){
    return Board_REG_in[IO_INch_KEY_LEFT];
}

void key_init(void){
    memset(&keystr,0,sizeof(keyStruct) *IO_INch_Max);

    for(unsigned char i = 0;i<IO_INch_Max;i++){
        keystr[i].Sdelay = KEYshortTIM;
        keystr[i].Ldelay = KEYlongTIM;
    }
    keystr[IO_INch_KEY_UP].pRfunc = &keyRead_key_up;
    keystr[IO_INch_KEY_RIGHT].pRfunc = &keyRead_key_right;
    keystr[IO_INch_KEY_DOWN].pRfunc  = &keyRead_key_down;
    keystr[IO_INch_KEY_LEFT].pRfunc  = &keyRead_key_left;
}

unsigned int keyXvalread(unsigned char ch,unsigned char valnum){
return  keyValread(&keystr[ch],valnum);
}

/* ==================== 按键事件锁存 (for UI 10ms refresh) ==================== */
static unsigned char g_key_event[IO_INch_Max] = {0};

//func
void key_func(void){
    unsigned char i;
    for(i = 0;i< IO_INch_Max;i++){
        keystr[i].u16tick++;
        keystr[i].u16Rtick++;
        keyShortPressCHK(&keystr[i]);
    }
    /* 锁存按键事件: 在key_func末尾捕获flag，供UI(10ms周期)读取 */
    for(i = 0;i< IO_INch_Max;i++){
        if(keystr[i].flag){
            g_key_event[i] = keystr[i].flag;
        }
    }
}

unsigned char key_event_read(unsigned char ch){
    unsigned char ev;
    if(ch >= IO_INch_Max) return 0;
    ev = g_key_event[ch];
    g_key_event[ch] = 0;   /* 读后清除 */
    return ev;
}

unsigned char key_state_read(unsigned char ch){
    if(ch >= IO_INch_Max) return 0;
    return keystr[ch].Pressing;
}