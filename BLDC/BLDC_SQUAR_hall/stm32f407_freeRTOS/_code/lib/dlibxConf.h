#ifndef __dlibxconf_h__ 
#define __dlibxconf_h__ 

#define TIM_1S_(x)  (x)
#define TIM_1MS_(x) (x)
#define TIM_1h_s(x) ((x)*3600)

#define bitx(x)    (1<<(x))

#define D_PORT_A    GpioPortA
#define D_PORT_B    GpioPortB
#define D_PORT_C    GpioPortC
#define D_PORT_D    GpioPortD
#define D_PORT_E    GpioPortE
#define D_PORT_F    GpioPortF

//#define D_PIN_(x)   bitx(x)
//#define D_AF_(x)    bitx(x)
#define D_PIN_(x)   (x)
#define D_AF_(x)    (x)

//#define RTTprintTEST

#ifdef RTTprintTEST
#define TASK_INFO_CHK
#endif

#define TIME_MS2HOUR(x)  ((x)*3600000)
#define TIME_MS2MIN(x)  ((x)*60000)
#define TIME_MS2SEC(x)  ((x)*1000)



typedef void(*pfunc) (void);
typedef unsigned char(*u8pfunc)(void);
//

//userdefine**********************************************
//1.key--------------------------------


//2.adc--------------------------------
//#define ADCNUM              2
//#define ADC_2ndTimes        3   //2^3

//#define AD_chgV     sADCx_data[0].avg
//#define AD_key      sADCx_data[1].avg

////3.led--------------------------------
//#define LEDNUM              2
//#define LED_FLASH_ONDELAY   500
//#define LED_FLASH_OFFDELAY  500

//#define LED_CH_W            0   //
//#define LED_CH_R            1   //
////end of userdefine**********************************************

#endif

