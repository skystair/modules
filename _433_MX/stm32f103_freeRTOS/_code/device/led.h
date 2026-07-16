#ifndef __led_h__ 
#define __led_h__ 

#define LED_FLASH_500MS 500
#define LED_FLASH_630MS 630
void LEDxinit(void);
void LEDxtick(void);
void LEDxfunc(void);

void LEDxCtrl(unsigned char sel,unsigned char state,unsigned short halfT);
#endif 

