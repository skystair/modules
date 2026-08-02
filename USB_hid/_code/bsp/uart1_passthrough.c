#include "uart1_passthrough.h"
#include "stm32f10x.h"

/* 接收环形缓冲区（中断写入，task 读出） */
#define RX_BUF_SIZE   512U
#define RX_BUF_MASK   (RX_BUF_SIZE - 1U)

static volatile uint8_t  rx_buf[RX_BUF_SIZE];
static volatile uint32_t rx_head = 0U;   /* ISR 写位置 */
static volatile uint32_t rx_tail = 0U;   /* task 读位置 */

/* 发送缓冲区（task 写入，中断逐字节发出） */
#define TX_BUF_SIZE   512U
#define TX_BUF_MASK   (TX_BUF_SIZE - 1U)

static volatile uint8_t  tx_buf[TX_BUF_SIZE];
static volatile uint32_t tx_head = 0U;   /* task 写位置 */
static volatile uint32_t tx_tail = 0U;   /* ISR 读位置 */
static volatile uint8_t  tx_busy = 0U;   /* 发送忙标志 */

/*==========================================================================
 * USART1 硬件初始化
 *==========================================================================*/

static void uart1_gpio_init(void) {
    /* PA9  = TX: 复用推挽输出 50MHz */
    GPIOA->CRH &= ~(0xFU << 4);
    GPIOA->CRH |=  (0xBU << 4);   /* CNF=10(AF_PP), MODE=11(50MHz) */
    /* PA10 = RX: 浮空输入 */
    GPIOA->CRH &= ~(0xFU << 8);
    GPIOA->CRH |=  (0x4U << 8);   /* CNF=01(Floating), MODE=00(Input) */
}

static void uart1_hw_init(uint32_t baud, uint8_t databits, uint8_t stopbits, uint8_t parity) {
    /* 开启时钟：USART1 + GPIOA */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;
    /* 复位 USART1 */
    RCC->APB2RSTR |=  RCC_APB2RSTR_USART1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;

    uart1_gpio_init();

    /* 波特率：USART1 在 APB2 = 72MHz */
    USART1->BRR = 72000000U / baud;

    /* 数据位：M 位 */
    uint32_t cr1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    if (databits == 9U) {
        cr1 |= USART_CR1_M;
    }

    /* 校验 */
    if (parity == 1U) {       /* 奇校验 */
        cr1 |= USART_CR1_PCE | USART_CR1_PS;
    } else if (parity == 2U) { /* 偶校验 */
        cr1 |= USART_CR1_PCE;
    }

    USART1->CR1 = cr1;

    /* 停止位 */
    uint32_t cr2 = 0U;
    if (stopbits == 2U) {
        cr2 |= USART_CR2_STOP_1;   /* 2 停止位 */
    }
    /* stopbits == 0 → 1 停止位，cr2 = 0 即可 */
    /* stopbits == 1 → 1.5 停止位（仅适用同步模式，CDC 不使用） */
    USART1->CR2 = cr2;

    /* NVIC */
    NVIC_SetPriority(USART1_IRQn, 3);
    NVIC_EnableIRQ(USART1_IRQn);

    /* 使能 USART */
    USART1->CR1 |= USART_CR1_UE;

    /* 清空接收缓冲 */
    rx_head = 0U;
    rx_tail = 0U;
    tx_head = 0U;
    tx_tail = 0U;
    tx_busy = 0U;
}

/*==========================================================================
 * 应用层 API
 *==========================================================================*/

void UART1_Passthrough_Init(void) {
    uart1_hw_init(115200U, 8U, 0U, 0U);
}

void UART1_Passthrough_SetConfig(uint32_t baud, uint8_t databits, uint8_t stopbits, uint8_t parity) {
    /* 先禁用 USART1 */
    USART1->CR1 &= ~USART_CR1_UE;
    NVIC_DisableIRQ(USART1_IRQn);

    /* 重新初始化硬件 */
    uart1_hw_init(baud, databits, stopbits, parity);
}

void UART1_Passthrough_Send(const uint8_t *data, uint32_t len) {
    uint32_t i;
    for (i = 0U; i < len; i++) {
        /* 等待发送数据寄存器空 */
        while (!(USART1->SR & USART_SR_TXE)) {}
        USART1->DR = data[i];
    }
    /* 等待最后一个字节发送完成 */
    while (!(USART1->SR & USART_SR_TC)) {}
}

uint32_t UART1_Passthrough_RxCount(void) {
    uint32_t h = rx_head;
    uint32_t t = rx_tail;
    return (h - t) & RX_BUF_MASK;
}

uint32_t UART1_Passthrough_Receive(uint8_t *buf, uint32_t max_len) {
    uint32_t count = 0U;
    while (count < max_len && rx_tail != rx_head) {
        buf[count++] = rx_buf[rx_tail];
        rx_tail = (rx_tail + 1U) & RX_BUF_MASK;
    }
    return count;
}

uint32_t UART1_Passthrough_Peek(uint8_t *buf, uint32_t max_len) {
    uint32_t count = 0U;
    uint32_t pos = rx_tail;
    while (count < max_len && pos != rx_head) {
        buf[count++] = rx_buf[pos];
        pos = (pos + 1U) & RX_BUF_MASK;
    }
    return count;
}

void UART1_Passthrough_Discard(uint32_t count) {
    rx_tail = (rx_tail + count) & RX_BUF_MASK;
}

/*==========================================================================
 * USART1 中断服务函数
 *==========================================================================*/

void USART1_IRQHandler(void) {
    uint32_t sr = USART1->SR;

    /* 接收数据就绪 */
    if (sr & USART_SR_RXNE) {
        uint8_t ch = (uint8_t)USART1->DR;
        uint32_t next = (rx_head + 1U) & RX_BUF_MASK;
        if (next != rx_tail) {   /* 缓冲区未满 */
            rx_buf[rx_head] = ch;
            rx_head = next;
        }
    }

    /* 发送数据寄存器空中断 */
    if ((USART1->CR1 & USART_CR1_TXEIE) && (sr & USART_SR_TXE)) {
        if (tx_tail != tx_head) {
            USART1->DR = tx_buf[tx_tail];
            tx_tail = (tx_tail + 1U) & TX_BUF_MASK;
        } else {
            /* 发送完毕，关闭 TXE 中断 */
            USART1->CR1 &= ~USART_CR1_TXEIE;
            tx_busy = 0U;
        }
    }

    /* 溢出错误清标志 */
    if (sr & USART_SR_ORE) {
        (void)USART1->DR;
    }
}