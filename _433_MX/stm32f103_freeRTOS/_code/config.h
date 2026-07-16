#ifndef __CONFIG_H
#define __CONFIG_H

#include "stm32f10x_conf.h"
//#include "stm32f1xx_hal.h"
#include "boardIO.h"

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

#include "RF_433_MX.h"
#include "RF_433_MX_TX.h"
#include "RF_433_MX_RX.h"

//#include "RF_433_MX_uart.h"


//#include "RF_433_MX_test_rx.h"
//#include "RF_433_MX_test_tx.h"
//#include "RF_433_MX_STM32F103.h"
//FUNCTION
#include "dlibx.h"
#include "dlibxConf.h"
//#define TRUE 1
//#define FALSE 0
//typedef enum {FALSE = 0,TRUE = 1} bool;
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "freertos_evr.h"
#include "task.h"
#include "timers.h"
//#include "FreeRTOSConfigxxx.h"


#include "task_mode.h"
#include "task_com1.h"





#endif
