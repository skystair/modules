#include "config.h"

LedStruct LEDstr[LEDNUM];

void LEDxDS0ctrl(unsigned char flag){
    Board_REG_out[IO_OUTch_LED_DS0] = (flag == 0);  // 低电平点亮
}
void LEDxDS1ctrl(unsigned char flag){
    Board_REG_out[IO_OUTch_LED_DS1] = (flag == 0);  // 低电平点亮
}
//void LEDxRctrl(unsigned char flag){
//    if(flag){
//        ON_LED_R;
//    }else{
//        OFF_LED_R;
//    }
//}

void LEDxinit(void){
    memset(&LEDstr,0,LEDNUM*sizeof(LedStruct));
    for(unsigned char i = 0; i< LEDNUM;i++){
        LEDstr[i].Flashondelay = LED_FLASH_ONDELAY;
        LEDstr[i].Flashoffdelay = LED_FLASH_OFFDELAY;
    }
    LEDstr[IO_OUTch_LED_DS0].pfunc = &LEDxDS0ctrl;
    LEDstr[IO_OUTch_LED_DS1].pfunc = &LEDxDS1ctrl;
}


void LEDxfunc(void){
    for(unsigned char i = 0; i< LEDNUM;i++){
        LEDstr[i].Flashtick++;
        LED_statefunc(&LEDstr[i]);
    }
}

void LEDxCtrl(unsigned char sel,unsigned char state,unsigned short halfT){
    if(sel >= LEDNUM) return;
    LEDstr[sel].state = state;
    if(state == LED_STATE_FLASH){
        LEDstr[sel].Flashondelay = halfT;
        LEDstr[sel].Flashoffdelay = halfT;
    }
}