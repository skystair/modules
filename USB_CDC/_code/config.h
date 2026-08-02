#ifndef __CONFIG_H
#define __CONFIG_H

#include "stm32f10x_conf.h"
//#include "stm32f1xx_hal.h"
#include "boarddef.h"

#include "stdio.h"
#include <stdlib.h> // malloc() free()
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
//bsp===============================
#include "timerx.h"
//device===============================
#include "key.h"
#include "led.h"

#include "usb_cdc_app.h"
#include "uart1_passthrough.h"

//FUNCTION
#include "dlibx.h"
#include "dlibxConf.h"
//#define TRUE 1
//#define FALSE 0
//typedef enum {FALSE = 0,TRUE = 1} bool;
#include "cmsis_os2.h"


#include "task_mode.h"
#include "task_com1.h"
#include "modefunc.h"

//#include "Driver_GPIO.h"


#endif
