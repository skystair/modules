#include "RF_433_MX.h"

void RF_433_MX_init(void) {
    RF_433_MX_TX_init();
    RF_433_MX_RX_init();
}

void RF_433_MX_tick1ms(void) {
    RF_433_MX_TX_tick1ms();
    /* RX 由 EXTI + TIM 中断驱动，无需在此轮询 */
}
