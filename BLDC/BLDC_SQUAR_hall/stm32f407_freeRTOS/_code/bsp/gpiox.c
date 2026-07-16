#include "gpiox.h" 

unsigned char IOoutCtrl[IO_OUTch_Max];
unsigned char IOinRead[IO_INch_Max];

void gpiox_init(void); 
void gpiox_func(void); 

/**
 * @brief GPIO初始化（基于board配置）
 * @note  新版本：使用board_v1.h中的配置数组自动初始化
 */
void gpiox_init(void){
    
}
void gpiox_func(void){
    unsigned char i;
//    for(i = 0;i<IO_OUTch_Max;i++){
//        if((i>=IO_OUTch_MotorAC)&&(i<=IO_OUTch_HeatAC)){
//            SetBit(((uint32_t)&M0P_GPIO->PAOUT + Board_GpioOutputs[i].port), Board_GpioOutputs[i].pin, (IOoutCtrl[i] == 0));
//        }else{
//            SetBit(((uint32_t)&M0P_GPIO->PAOUT + Board_GpioOutputs[i].port), Board_GpioOutputs[i].pin, (IOoutCtrl[i] != 0));
//        }
//    }
//    
//    for(i = 0;i < keych_Max;i++){
//        IOinRead[i] = Board_GpioInputs[i].defaultState != GetBit(((uint32_t)&M0P_GPIO->PAIN + Board_GpioInputs[i].GPIO.port),  Board_GpioInputs[i].GPIO.pin);
//    }
}