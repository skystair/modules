/**
 ******************************************************************************
 * @file    board_v1.c
 * @brief   MCU底层硬件配置实现 - Board V1版本
 * @details 本文件根据board_v1.h中的配置信息初始化所有MCU底层外设
 ******************************************************************************
 */
#include "config.h"

unsigned char Board_REG_out[IO_OUTch_Max];
unsigned char Board_REG_in[IO_INch_Max];
/**
 * @brief ADC通道配置数组
 * @note  索引0-12对应sADCx_data[0]-sADCx_data[12]
 */
//const BoardAdcChannelConfig_t Board_AdcChannels[ADCch_Max] = {
//     //HALL_OUT (PA02)
//    {{D_PORT_A, D_PIN_(2)}, AdcExInputCH2},
//};

/* ============================================================================
 *                        GPIO配置实现
 * ============================================================================ */
/**
 * @brief GPIO输出引脚配置数组
 * @note  数组索引对应IOoutStr[]的下标
 */
const BoardGpioConfig_t Board_GpioOutputs[IO_OUTch_Max] = {
    // 索引0: WIFI_RELOAD (PB13)
    {D_PORT_B, D_PIN_(5),1},
	{D_PORT_E, D_PIN_(5),1},
};

/**
 * @brief GPIO输入引脚配置数组
 */
const BoardGpioConfig_t Board_GpioInputs[IO_INch_Max] = {
    // KEY_USR (PC13)
    {D_PORT_E, D_PIN_(4),0},
    // KEY_WAKEUP (PA0)
    {D_PORT_E, D_PIN_(3),0},
	{D_PORT_E, D_PIN_(2),0},
	{D_PORT_A, D_PIN_(0),0},
};

/* ============================================================================
 *                        UART配置实现
 * ============================================================================ */

/**
 * @brief UART配置数组
 * @note  索引对应BoardUartId_t枚举值
 */
//const BoardUartConfig_t Board_Uarts[BOARD_N_UART_NUM] = {
//    // BOARD_UART_WIFI: UART0 (PA09-TX, PA10-RX)
//    {
//        .ioTX.port = D_PORT_A,
//        .ioTX.pin = D_PIN_(9),
//        .ioTX.Af = D_AF_(1),
//        .ioRX.port = D_PORT_A,
//        .ioRX.pin = D_PIN_(10),
//        .ioRX.Af = D_AF_(1),
//        .uartBase = M0P_UART0,
//        .irqNum = UART0_2_IRQn
//    }
//};
//const BoardLUartConfig_t Board_LUarts[BOARD_L_UART_NUM] = {
//    // BOARD_UART_HANDCTRL: LPUART0 (PC10-TX, PC11-RX)
//    {
//        .ioTX.port = D_PORT_C,
//        .ioTX.pin = D_PIN_(10),
//        .ioTX.Af = D_AF_(2),
//        .ioRX.port = D_PORT_C,
//        .ioRX.pin = D_PIN_(11),
//        .ioRX.Af = D_AF_(2),
//        .uartBase = M0P_LPUART0,
//        .irqNum = LPUART0_IRQn
//    },
//    // BOARD_UART_BACKBAT: LPUART1 (PA00-TX, PA01-RX)
//    {
//        .ioTX.port = D_PORT_A,
//        .ioTX.pin = D_PIN_(0),
//        .ioTX.Af = D_AF_(2),
//        .ioRX.port = D_PORT_A,
//        .ioRX.pin = D_PIN_(1),
//        .ioRX.Af = D_AF_(2),
//        .uartBase = M0P_LPUART1,
//        .irqNum = LPUART1_IRQn
//    }
//};
/* ============================================================================
 *                        Timer/PWM配置实现
 * ============================================================================ */

/**
 * @brief PWM配置数组
 * @note  数组索引对应PWM_Str[]的下标（0-9）
 */
//const BoardPWMConfig_t Board_Pwms[nPWM_Max] = {
//    // 索引0: nPWM_collectdoor_L (TIM0-CHA, PB14)
//    {
//        .GPIO = {D_PORT_B, D_PIN_(14), D_AF_(4)},
//        .CCR = &M0P_TIM0_MODE23->CCR0A
//    },
//};

/* ============================================================================
 *                        SPI配置实现
 * ============================================================================ */

/**
 * @brief SPI配置数组
 */
//const BoardSpiConfig_t Board_Spis[BOARD_SPI_NUM] = {
//    // BOARD_SPI_RGB: SPI0 (PA12-DOUT, DMA Channel 1)
//    {
//        .GPIO.port = D_PORT_A,
//        .GPIO.pin = D_PIN_(12),
//        .GPIO.Af = D_AF_(6),
//        .spiBase = M0P_SPI0,
//        .dmaCh = DmaCh1
//    }
//};

void gpiox_init(void){
    unsigned char i;
    GPIO_InitTypeDef    GPIO_str;
    GPIO_str.GPIO_Speed = GPIO_Speed_10MHz;
    
    GPIO_str.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC,&GPIO_str);
    
    GPIO_str.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
    GPIO_Init(GPIOA,&GPIO_str);
    
    GPIO_str.GPIO_Mode = GPIO_Mode_Out_PP;
    //out
    for(i = 0;i < IO_OUTch_Max;i++){
        GPIO_str.GPIO_Pin = Board_GpioOutputs[i].pin;
        if(Board_GpioOutputs[i].defautLV){
            Board_REG_out[i] = 1;
            Board_GpioOutputs[i].port->BSRR = Board_GpioOutputs[i].pin;
        }else{
            Board_REG_out[i] = 0;
            Board_GpioOutputs[i].port->BRR = Board_GpioOutputs[i].pin;
        }
        GPIO_Init(Board_GpioOutputs[i].port,&GPIO_str);
    }
    GPIO_str.GPIO_Mode = GPIO_Mode_IPD;//GPIO_Mode_IN_FLOATING;
    //in
    for(i = 0;i < IO_INch_Max;i++){
        GPIO_str.GPIO_Pin = Board_GpioInputs[i].pin;
        GPIO_Init(Board_GpioInputs[i].port,&GPIO_str);
    }
}
void gpiox_func(void){
    unsigned char i;

    //out: REG_out控制对应引脚电平
    for(i = 0;i < IO_OUTch_Max;i++){
        if(Board_REG_out[i]){
            Board_GpioOutputs[i].port->BSRR = Board_GpioOutputs[i].pin;
        }else{
            Board_GpioOutputs[i].port->BRR = Board_GpioOutputs[i].pin;
        }
    }

    //in: 读取引脚电平存入REG_in
    for(i = 0;i < IO_INch_Max;i++){
        Board_REG_in[i] = Board_GpioInputs[i].defautLV != GPIO_ReadInputDataBit(Board_GpioInputs[i].port, Board_GpioInputs[i].pin);
    }
}
/* ============================================================================
 *                        初始化函数实现
 * ============================================================================ */

void Board_Init(void){
    gpiox_init();
//    adcx_init();
//    uartx_init();
//    spix_init();
}

void Board_func(void){
    gpiox_func();
}