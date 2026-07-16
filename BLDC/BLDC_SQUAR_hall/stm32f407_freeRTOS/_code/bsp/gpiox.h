#ifndef __gpiox_h__ 
#define __gpiox_h__ 

//#include ""
typedef enum {
    IO_OUTch_LEDG = 0,
    IO_OUTch_LEDR,
    IO_OUTch_BLDC_PWR,
    IO_OUTch_BLDC_lbU,
    IO_OUTch_BLDC_lbV,
    IO_OUTch_BLDC_lbW,
    IO_OUTch_Max            ///< 引脚总数
} io_out_ch_t;

typedef enum {
    IO_INch_BLDC_hallU,
    IO_INch_BLDC_hallV,
    IO_INch_BLDC_hallW,
    IO_INch_Max            ///< 引脚总数
} io_in_ch_t;

typedef struct {
//    GPIO_TypeDef  *GPIOx;      ///< GPIO端口
//    en_gpio_pin_t  pin;       ///< GPIO引脚
////    boolean_t      defaultLevel;  ///< 默认电平（TRUE=高，FALSE=低）
} BoardGpioConfig_t;






#endif 
