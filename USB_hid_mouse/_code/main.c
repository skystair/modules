#include "config.h"

void main_init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

    Board_Init();
    tick_init();
    key_init();
    LEDxinit();

    modefunc_init();
}

int main(void){
    main_init();

    osKernelInitialize();

    USB_HID_Init();

    task_mode_creat();
    task_com1_creat();

    osKernelStart();
}
