#include "config.h"

modestruct modestr;
extern LedStruct LEDstr[LEDNUM];
/* ====================== 状态机前置/全局/后置 ====================== */
static void modefunc_Prefunc(void){
    unsigned char i;
    //只清原始输出层，驱动state由各mode handler显式管理
    memset(Board_REG_out, 0, IO_OUTch_Max);
    
    //drive
    for(i = 0; i< LEDNUM;i++){
        LEDstr[i].state = 0;
    }
}

static void modefunc_Pubfunc(void){
//    if(keyXvalread(IO_INch_KEY_USR,KEY_VAL_FLAG)){
//        //usb send
//    }
    if(modestr.state != modestr.statelast){
        //状态切换: 离开旧状态
        switch(modestr.statelast){
            case modech_idle:   break;
            case modech_func1:  break;
            case modech_func2:  break;
            default: break;
        }
        //进入新状态
        switch(modestr.state){
            case modech_reset:  break;
            case modech_idle:   break;
            case modech_func1:  break;
            case modech_func2:  break;
            default: break;
        }
        modestr.statelast = modestr.state;
    }
}

static void modefunc_Lastfunc(void){
    unsigned char left_pressing = keyXvalread(KEY_CH_trig, KEY_VAL_PRESS);
    unsigned char right_pressing = keyXvalread(KEY_CH_wakeup, KEY_VAL_PRESS);
    USB_HID_Mouse_Func(left_pressing, right_pressing);
}

/* ====================== 各状态处理函数 ====================== */
static void modefunc_reset(void){
    //复位态: 初始化完成后直接切到idle
    modestr.state = modech_idle;
}

static void modefunc_idle(void){
    LEDxCtrl(IO_OUTch_LED_R,2,500);
}

static void modefunc_func1(void){

}

static void modefunc_func2(void){

}

/* ====================== 状态机调度表 ====================== */
pfunc modefunclist[modech_MAX] = {
    modefunc_reset,
    modefunc_idle,
    modefunc_func1,
    modefunc_func2,
};

/* ====================== 初始化 ====================== */
void modefunc_init(void){
    memset(&modestr, 0, sizeof(modestruct));
    modestr.state = modech_reset;
    modestr.statelast = modech_MAX;
}

void modefunc_setState(unsigned char newState){
    if(newState < modech_MAX){
        modestr.state = newState;
    }
}

/* ====================== 周期调用 ====================== */
void modefunc_tick1ms(void){
    if(modestr.tick < 0xFFFFFFFF){
        modestr.tick++;
    }
}

void modefunc_func(void){
    modefunc_tick1ms();

    modefunc_Prefunc();
    modefunc_Pubfunc();

    if(modestr.state < modech_MAX){
        modefunclist[modestr.state]();
    }

    modefunc_Lastfunc();
}
