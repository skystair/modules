#ifndef __gpiox_h__
#define __gpiox_h__

typedef enum {
    IO_OUTch_LED0 = 0,
    IO_OUTch_LED1,
    IO_OUTch_Max            ///< 引脚总数
} io_out_ch_t;

typedef enum {
    IO_INch_BLDC_hallU,
    IO_INch_BLDC_hallV,
    IO_INch_BLDC_hallW,
    IO_INch_Max            ///< 引脚总数
} io_in_ch_t;

/* 全局变量声明 */
extern unsigned char IOoutCtrl[];
extern unsigned char IOinRead[];

/* 函数声明 */
void gpiox_init(void);
void gpiox_func(void);

#endif /* __gpiox_h__ */ 
