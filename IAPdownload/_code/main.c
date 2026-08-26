#include "config.h"

void main_init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);  /* LCD数据总线PD */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);  /* LCD_CS(PG12), LCD_RS(PG0) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC,ENABLE);     /* FSMC时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

    Board_Init();
    USB_CDC_Init();
    UART1_Passthrough_Init();
    
    tick_init();
    key_init();
    LEDxinit();
    
    lcd_init();     /* LCD硬件初始化(FSMC+GPIO+ID读取) */
    /* LCD软件初始化移到task_com1中分步执行，避免阻塞 */
    
    flash_store_init();
    transfer_init();
    
    ui_init();      /* UI初始化(lcd_init和flash_store_init之后) */
    
    modefunc_init();
    
}

int main(void){
    main_init();

    osKernelInitialize();

    

    task_mode_creat();
    task_com1_creat();

    osKernelStart();
}
