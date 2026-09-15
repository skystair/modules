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


#endif

 