#include "dlibx.h"

unsigned int GetSUM(unsigned char* str,unsigned char p1,unsigned char p2){
    unsigned int ret = 0;
    while(p1 <= p2){
        ret+=str[p1++];
    }
    return ret;
}
//elem_size = sizeof(char/short/int)
unsigned int CalSUM_unsign(const void* addrST,unsigned char elem_size,unsigned int length){
    unsigned int ret = 0,i;
    const unsigned char* addr_sys = (const unsigned char*)addrST;
    const void* addr_final;
    for(i = 0;i<length;i++){
        addr_final = addr_sys + i * elem_size;
        switch(elem_size){
            case 1:ret += *(const unsigned char*)addr_final;break;
            case 2:ret += *(const unsigned short*)addr_final;break;
            case 4:ret += *(const unsigned int*)addr_final;break;
            default:break;
        }
    }
    return ret;
}
int CalSUM_sign(const void* addrST,unsigned char elem_size,unsigned int length){
    int ret = 0,i;
    const char* addr_sys = (const char*)addrST;
    const void* addr_final;
    for(i = 0;i<length;i++){
        addr_final = addr_sys + i * elem_size;
        switch(elem_size){
            case 1:ret += *(const char*)addr_final;break;
            case 2:ret += *(const short*)addr_final;break;
            case 4:ret += *(const int*)addr_final;break;
            default:break;
        }
    }
    return ret;
}
//0.uartxfunc========================================================
void uartx_Rxclear(STRUart *Uartstruct){
    Uartstruct->Rxp = 0;
    Uartstruct->Rxtmp = 0;
    Uartstruct->Rxcompflag = 0;
}

void Uart_TXIT_func(STRUart *Uartstruct){
    if(Uartstruct->TXing == 1){
        Uartstruct->Txtick = 0;
        
        Uartstruct->fpWaitTxend();
        if(Uartstruct->TXp >= Uartstruct->TXlen){
            //等发送完成
            Uartstruct->TXing = 0;
            Uartstruct->TXp = 0;
            //关tx
            Uartstruct->fpTxINT_ENctrl(0);
            if(Uartstruct->RenAfterTx){
                Uartstruct->fpRxen_ENctrl(1);
            }
            Uartstruct->TXcmp = 1;
        }else{
            //发送
            Uartstruct->fpTxbyte(Uartstruct->Txbuff[Uartstruct->TXp++]);
        }
    }
}
void Uart_RXIT_func(STRUart *Uartstruct){
    if(Uartstruct->Rxcompflag) return;
    
    if(Uartstruct->Rxp < Uartstruct->RxchkLen){//特定数据---------------------
        if(Uartstruct->Rxtmp == Uartstruct->CHKbuff[Uartstruct->Rxp]){
            Uartstruct->Rxbuff[Uartstruct->Rxp++] = Uartstruct->Rxtmp;
        }else if(Uartstruct->Rxtmp == Uartstruct->CHKbuff[0]){
            Uartstruct->Rxp = 1;
        }else{
            uartx_Rxclear(Uartstruct);
        }
    }else if(Uartstruct->Rxp < Uartstruct->RxLen){//无条件数据------------------
        Uartstruct->Rxbuff[Uartstruct->Rxp++] = Uartstruct->Rxtmp;
    }
    
    if(Uartstruct->Rxp >= Uartstruct->RxLen){//超预计长度后完成----------------------
        Uartstruct->Rxcompflag = 1;
        Uartstruct->Rxp = 0;
    }
}
void Uart_RXIT_funcL(STRUart *Uartstruct,unsigned char pL){
    if(Uartstruct->Rxcompflag) return;
    
    if(Uartstruct->Rxp < Uartstruct->RxchkLen){//特定数据---------------------
        if(Uartstruct->Rxtmp == Uartstruct->CHKbuff[Uartstruct->Rxp]){
            Uartstruct->Rxbuff[Uartstruct->Rxp++] = Uartstruct->Rxtmp;
        }else if(Uartstruct->Rxtmp == Uartstruct->CHKbuff[0]){
            Uartstruct->Rxp = 1;
        }else{
            uartx_Rxclear(Uartstruct);
        }
    }else if(Uartstruct->Rxp < Uartstruct->RxLen){//无条件数据------------------
        if(Uartstruct->Rxp == pL){
            if(Uartstruct->Rxtmp > Uart_RXIT_RXL_MAX){
                if(Uartstruct->Rxtmp == Uartstruct->CHKbuff[0]){
                    Uartstruct->Rxp = 1;
                }else{
                    uartx_Rxclear(Uartstruct);
                    return;
                }
            }else{
                Uartstruct->RxLen = Uartstruct->Rxtmp + 5;
            }
        }
        Uartstruct->Rxbuff[Uartstruct->Rxp++] = Uartstruct->Rxtmp;
    }
    
    if(Uartstruct->Rxp >= Uartstruct->RxLen){//超预计长度后完成----------------------
        Uartstruct->Rxcompflag = 1;
        Uartstruct->Rxp = 0;
    }
}
//1.adcFunc==========================================================
void adcx_Getvalue_func(ADCx_logicStruct* adclStr,ADCx_dataStruct* adcdStr){
    if(!adclStr->u8cmpflag) return;
    adclStr->u8cmpflag = 0;
    
    if(!adclStr->u8first){
        adclStr->u8first = 1;
        //首次采样
        for(unsigned char i= 0;i < adclStr->ADnums;i++){
            adcdStr[i].avg = adcdStr[i].real;
            adcdStr[i].u32sum = adcdStr[i].real <<(adclStr->PWRtimes);
            
            if(adcdStr[i].u32sum > adcdStr[i].real){
                adcdStr[i].u32sum-= adcdStr[i].real;
            }else{
                adcdStr[i].u32sum = 0;
            }
        }
        return;
    }
    //加入新值求平均
    for(unsigned char i = 0;i < adclStr->ADnums;i++){
        adcdStr[i].u32sum += adcdStr[i].real;   //加入新值
        adcdStr[i].avg = adcdStr[i].u32sum >> (adclStr->PWRtimes);   //求平均
        adcdStr[i].u32sum -= adcdStr[i].avg;    //去掉一个上轮平均值
    }
}

//2.keyFunc==========================================================
void keyShortPressCHK(keyStruct *keys){
    if(keys->flag){
        keys->flag = 0;
    }
    keys->Pressing = keys->pRfunc();
    if(keys->Pressing){
        if(keys->u16tick >= keys->Sdelay){
            if(keys->u16tick >= 0xff00){
                keys->u16tick = 0xff00;
            }
            if(keys->keep != 1){
                keys->keep = 1;
                keys->flag = KEY_FALG_SHORT;
            }
        }
        keys->u16Rtick = 0;
    }else{
//		keys->flag = 0;
        keys->keep = 0;
        keys->u16tick = 0;
        if(keys->u16Rtick >= 0xff00){
            keys->u16Rtick = 0xff00;
        }
    }
}

void keyLongPressCHK(keyStruct *keys){
    if(keys->flag){
        keys->flag = 0;
    }
    keys->Pressing = keys->pRfunc();
    if(keys->Pressing){
        if(keys->u16tick >= keys->Ldelay){
            if(keys->u16tick >= 0xff00){
                keys->u16tick = 0xff00;
            }
            if(keys->keep != 1)
            {
                keys->keep = 1;
                keys->flag = KEY_FALG_LONG;
            }
        }
        keys->u16Rtick = 0;
    }else{
        if(keys->u16tick <= keys->Sdelay){
//            keys->flag = 0;
        }else if(keys->u16tick < keys->Ldelay){
            keys->flag = KEY_FALG_SHORT;
        }
        
        keys->keep = 0;
        keys->u16tick = 0;
        if(keys->u16Rtick >= 0xff00){
            keys->u16Rtick = 0xff00;
        }
    }
}

void keyflagclr(keyStruct* keyx){
    keyx->flag = 0;
}
unsigned int keyValread(keyStruct* keyx,unsigned char valnum){
    unsigned int ret = 0;
    switch(valnum){
        case KEY_VAL_PRESS:
            ret = keyx->Pressing;
            break;
        case KEY_VAL_FLAG:
            ret = keyx->flag;
            break;
        case KEY_VAL_KEEP:
            ret = keyx->keep;
            break;
        case KEY_VAL_DELAY:
            ret = keyx->u16tick;
            break;
        case KEY_VAL_RDELAY:
            ret = keyx->u16Rtick;
            break;
        default:break;
    }
    return ret;
}

//3.ledFunc==========================================================
void LED_statefunc(LedStruct* LEDstr){
    switch(LEDstr->state){
        case LED_STATE_OFF:
            LEDstr->onflag = 0;
            LEDstr->Flashtick = 0;
            break;
        case LED_STATE_ON:
           LEDstr->onflag = 1;
            LEDstr->Flashtick = LEDstr->Flashondelay;
            break;
        case LED_STATE_FLASH:
            if(LEDstr->Flashtick < LEDstr->Flashondelay){
                LEDstr->onflag = 1;
            }else if(LEDstr->Flashtick < (LEDstr->Flashondelay + LEDstr->Flashoffdelay)){
                LEDstr->onflag = 0;
            }else{
                LEDstr->Flashtick = 0;
            }
            break;
        default:
            LEDstr->onflag = 0;
            break;
    }
    LEDstr->pfunc(LEDstr->onflag);
}

//4.xxxFunc==========================================================
void valjudgefunc(unsigned char tf,unsigned short* tick,unsigned short delay,unsigned char* flag){
    if(tf){
        if(*tick >= delay){
            *tick = delay;
            *flag = 1;
        }
    }else{
        *tick = 0;
        *flag = 0;
    }
}
void valjudgeNcoverfunc(unsigned char tf,unsigned short* tick,unsigned short delay,unsigned char* flag){
    if(tf){
        if(*tick >= delay){
            *tick = delay;
            *flag = 1;
        }
    }else{
        *tick = 0;
        // *flag = 0;
    }
}
void valJandRcoverfunc(unsigned char tf1,unsigned char tf2,unsigned short* tick,unsigned short delay,unsigned char* flag){
    if(tf1){
        if(*tick >= delay){
            *tick = delay;
            *flag = 1;
        }
    }else{
        if(tf2){
            *flag = 0;
        }
        *tick = 0;
    }
}

