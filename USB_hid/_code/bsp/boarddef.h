/**
 &zwnj;******************************************************************************&zwnj;
 * @file    board_v1.h
 * @brief   MCU底层硬件配置头文件 - Board V1版本
 * @details 本文件集中定义HC32F17x系列MCU的底层硬件资源配置
 *          - 仅描述MCU物理引脚和片上外设，不涉及具体业务用途
 *          - 外设用途应在各自驱动模块中通过宏定义声明
 *          - 支持ADC、GPIO、UART、Timer/PWM、SPI等外设配置
 &zwnj;******************************************************************************&zwnj;
 */

#ifndef __BOARDDEF_H__
#define __BOARDDEF_H__

#include "stm32f10x.h"

#define D_PORT_A    GPIOA
#define D_PORT_B    GPIOB
#define D_PORT_C    GPIOC
#define D_PORT_D    GPIOD
#define D_PORT_E    GPIOE
#define D_PORT_F    GPIOF
#define D_PORT_G    GPIOG

#define D_PIN_(x)    (1<<(x))

typedef enum {
    IO_OUTch_LED_R = 0,
	IO_OUTch_LED_G,
    IO_OUTch_Max            ///< 引脚总数
} io_out_ch_t;
#define LEDNUM  (IO_OUTch_LED_G+1)
typedef enum {
    IO_INch_KEY_R = 0,
    IO_INch_KEY_M,
	IO_INch_KEY_L,
	IO_INch_KEY_UP,
    IO_INch_Max            ///< 引脚总数
} io_in_ch_t;

/* --------------------------------------------------------------------------
 * Board-level ADC channel index macros
 * -------------------------------------------------------------------------- */
typedef enum {
    ADCch_REV1 = 0,
    ADCch_REV2,
    ADCch_Max             ///< ADC通道总数
} adc_ch_t;


typedef enum {
    nPWM_rev = 0,

    nPWM_Max                  ///< PWM通道总数
} pwm_ch_t;

//=====================================================================================
/**
 * @brief GPIO输出引脚配置结构
 */
typedef struct {
    GPIO_TypeDef*   port;      ///< GPIO端口
    unsigned short  pin;       ///< GPIO引脚
    unsigned char   defautLV;
} BoardGpioConfig_t;

/**
 * @brief ADC通道引脚映射结构
 */
//typedef struct {
//    BoardGpioConfig_t       GPIO;
//    en_adc_samp_ch_sel_t    chx; ///< ADC内部通道号
//} BoardAdcChannelConfig_t;

//typedef struct {
//    en_gpio_port_t port;      ///< GPIO端口
//    en_gpio_pin_t  pin;       ///< GPIO引脚
//    unsigned char  Af;        ///< 复用功能
//} BoardGpioAFConfig_t;

/**
 * @brief PWM通道配置结构（扩展AF配置，增加CCR寄存器指针）
 */
//typedef struct {
//    BoardGpioAFConfig_t GPIO;           ///< GPIO复用配置
//    volatile unsigned int*  CCR;            ///< CCR寄存器地址指针
//} BoardPWMConfig_t;
/**
 * @brief UART接口ID枚举
 */
//typedef enum {
//    BOARD_UART_WIFI = 0,      ///< WiFi模块UART
//} BoardUartId_t;

//typedef enum {
//    BOARD_UART_HANDCTRL = 0,  ///< 手柄控制UART
//    BOARD_UART_BACKBAT = 1,   ///< 后备电池UART
//} BoardLUartId_t;

/**
 * @brief UART引脚配置结构
 */
//typedef struct {
//    BoardGpioAFConfig_t ioTX;
//    BoardGpioAFConfig_t ioRX;
//    
//    M0P_UART_TypeDef* uartBase;  ///< UART基地址指针
//    IRQn_Type irqNum;         ///< 中断号
//} BoardUartConfig_t;

//typedef struct {
//    BoardGpioAFConfig_t ioTX;
//    BoardGpioAFConfig_t ioRX;
//    
//    M0P_LPUART_TypeDef* uartBase;  ///< UART基地址指针
//    IRQn_Type irqNum;         ///< 中断号
//} BoardLUartConfig_t;
///**
// * @brief SPI接口ID枚举
// */
//typedef enum {
//    BOARD_SPI_RGB = 0,        ///< RGB灯带SPI
//} BoardSpiId_t;

///**
// * @brief SPI引脚配置结构
// */
//typedef struct {
//    BoardGpioAFConfig_t   GPIO;
//    
//    M0P_SPI_TypeDef* spiBase; ///< SPI基地址指针
//    en_dma_channel_t dmaCh;   ///< DMA通道号
//} BoardSpiConfig_t;


extern const BoardGpioConfig_t          Board_GpioOutputs[IO_OUTch_Max];
extern const BoardGpioConfig_t          Board_GpioInputs[IO_INch_Max];
extern unsigned char Board_REG_out[IO_OUTch_Max];
extern unsigned char Board_REG_in[IO_INch_Max];
//extern const BoardGpioINConfig_t        Board_GpioInputs[keych_Max];
//extern const BoardAdcChannelConfig_t    Board_AdcChannels[ADCch_Max];
//extern const BoardSpiConfig_t           Board_Spis[BOARD_SPI_NUM];
//extern const BoardPWMConfig_t           Board_Pwms[nPWM_Max];
//extern const BoardUartConfig_t          Board_Uarts[BOARD_N_UART_NUM];
//extern const BoardLUartConfig_t         Board_LUarts[BOARD_L_UART_NUM];

/* ============================================================================
 *                        初始化接口
 * ============================================================================ */

/**
 * @brief 板级硬件初始化
 */
void Board_Init(void);

/**
 * @brief 板级周期性处理（GPIO输入/输出刷新）
 */
void Board_func(void);
#endif /* __BOARD_V1_H__ */
