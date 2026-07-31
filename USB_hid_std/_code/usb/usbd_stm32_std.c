/*
 * USB Device Driver for STM32F103 - StdPeriph implementation
 * Implements ARM_DRIVER_USBD interface for MDK-Middleware USB stack
 *
 * PMA addressing: each 16-bit PMA word occupies 4 bytes in ARM address space.
 * Descriptor table fields (ADDR_TX, COUNT_TX, ADDR_RX, COUNT_RX) are at
 * 4-byte intervals. ADDR values are PMA word addresses (byte_offset / 4).
 */
#include "stm32f10x.h"
#include "Driver_USBD.h"
#include <string.h>

/* StdPeriph 中密度设备可能未定义 USB 外设，回退定义 */
#ifndef USB_BASE
  #define USB_BASE        (APB1PERIPH_BASE + 0x00005C00UL)
#endif

#ifndef USB_TypeDef
typedef struct {
    __IO uint16_t EP0R;           /* 0x00 */
    __I  uint16_t _r0;
    __IO uint16_t EP1R;           /* 0x04 */
    __I  uint16_t _r1;
    __IO uint16_t EP2R;           /* 0x08 */
    __I  uint16_t _r2;
    __IO uint16_t EP3R;           /* 0x0C */
    __I  uint16_t _r3;
    __IO uint16_t EP4R;           /* 0x10 */
    __I  uint16_t _r4;
    __IO uint16_t EP5R;           /* 0x14 */
    __I  uint16_t _r5;
    __IO uint16_t EP6R;           /* 0x18 */
    __I  uint16_t _r6;
    __IO uint16_t EP7R;           /* 0x1C */
    __I  uint16_t _r7[17];       /* 0x20-0x3F reserved (17 half-words) */
    __IO uint16_t CNTR;           /* 0x40 */
    __I  uint16_t _r8;
    __IO uint16_t ISTR;           /* 0x44 */
    __I  uint16_t _r9;
    __IO uint16_t FNR;            /* 0x48 */
    __I  uint16_t _rA;
    __IO uint16_t DADDR;          /* 0x4C */
    __I  uint16_t _rB;
    __IO uint16_t BTABLE;         /* 0x50 */
    __I  uint16_t _rC;
    __IO uint16_t LPMCSR;         /* 0x54 */
    __I  uint16_t _rD;
    __IO uint16_t BCDR;           /* 0x58 */
    __I  uint16_t _rE;
} USB_TypeDef;
#endif

#ifndef USB
  #define USB             ((USB_TypeDef *)USB_BASE)
#endif

#ifndef USB_BCDR_DPPU
  #define USB_BCDR_DPPU   ((uint16_t)0x8000)
#endif
#ifndef USB_DADDR_EF
  #define USB_DADDR_EF    ((uint16_t)0x0080)
#endif
#ifndef USB_FNR_FN
  #define USB_FNR_FN      ((uint16_t)0x07FF)
#endif

/* ====================== PMA 缓冲区描述符 ====================== */
typedef struct {
    __IO uint16_t ADDR_TX;
    __I  uint16_t _pad0;
    __IO uint16_t COUNT_TX;
    __I  uint16_t _pad1;
    __IO uint16_t ADDR_RX;
    __I  uint16_t _pad2;
    __IO uint16_t COUNT_RX;
    __I  uint16_t _pad3;
} PMA_BufDesc_t;

#define PMA_BASE       0x40006000UL
#define PMA_BTABLE     ((PMA_BufDesc_t *)PMA_BASE)
#define EP_COUNT       8
#define EP_DIR_IN      0x80U

/* ====================== EPnR 位定义 ====================== */
#define EP_CTR_RX      0x8000U
#define EP_DTOG_RX     0x4000U
#define EPRX_STAT      0x3000U
#define EP_SETUP       0x0800U
#define EP_T_FIELD     0x0600U
#define EP_KIND        0x0100U
#define EP_CTR_TX      0x0080U
#define EP_DTOG_TX     0x0040U
#define EPTX_STAT      0x0030U
#define EP_ADDR_FIELD  0x000FU

/* EP_TYPE at bits [10:9]: 00=Bulk, 01=Control, 10=Iso, 11=Interrupt */
#define EP_T_BULK      0x0000U
#define EP_T_CONTROL   0x0200U
#define EP_T_ISO       0x0400U
#define EP_T_INT       0x0600U

#define EP_RX_DIS      0x0000U
#define EP_RX_STALL    0x1000U
#define EP_RX_NAK      0x2000U
#define EP_RX_VALID    0x3000U

#define EP_TX_DIS      0x0000U
#define EP_TX_STALL    0x0010U
#define EP_TX_NAK      0x0020U
#define EP_TX_VALID    0x0030U

#define EP_INVARIANT   (EP_CTR_RX | EP_CTR_TX | EP_T_FIELD | EP_KIND | EP_ADDR_FIELD)

/* CNTR */
#define CNTR_CTRM      0x8000U
#define CNTR_PMAOVRM   0x4000U
#define CNTR_ERRM      0x2000U
#define CNTR_WKUPM     0x1000U
#define CNTR_SUSPM     0x0800U
#define CNTR_RESETM    0x0400U
#define CNTR_FSUSP     0x0008U
#define CNTR_PDWN      0x0002U
#define CNTR_FRES      0x0001U

/* ISTR */
#define ISTR_CTR       0x8000U
#define ISTR_PMAOVR    0x4000U
#define ISTR_ERR       0x2000U
#define ISTR_WKUP      0x1000U
#define ISTR_SUSP      0x0800U
#define ISTR_RESET     0x0400U
#define ISTR_DIR       0x0010U
#define ISTR_EP_ID     0x000FU

/* ====================== 端点信息 ====================== */
typedef struct {
    uint8_t  *buf;
    uint32_t  num;
    uint32_t  transferred;
    uint16_t  max_packet_size;
    uint16_t  type;
    uint8_t   configured;
} EP_Info_t;

static EP_Info_t ep_info[EP_COUNT][2]; /* [ep][0=OUT,1=IN] */

/* Setup data buffer - read in ISR to avoid PMA race */
static uint8_t setup_buf[8];

/* ====================== 驱动状态 ====================== */
static ARM_USBD_SignalDeviceEvent_t   cb_device_event;
static ARM_USBD_SignalEndpointEvent_t cb_endpoint_event;
static uint8_t  device_address;
static uint8_t  device_connected;
static uint16_t pma_alloc_off;

#define EP0_PMA_RX_OFF  128
#define EP0_PMA_TX_OFF  192
#define EP0_PMA_END     256

/* ====================== PMA 读写 ====================== */
static void PMA_Write(uint16_t pma_byte_off, const uint8_t *data, uint16_t len) {
    __IO uint16_t *pma = (__IO uint16_t *)(PMA_BASE + pma_byte_off);
    while (len > 1) {
        *pma = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
        data += 2;
        len  -= 2;
        pma  += 2;
    }
    if (len == 1) {
        *pma = (*pma & 0xFF00U) | data[0];
    }
}

static void PMA_Read(uint16_t pma_byte_off, uint8_t *data, uint16_t len) {
    __IO uint16_t *pma = (__IO uint16_t *)(PMA_BASE + pma_byte_off);
    while (len > 1) {
        uint16_t val = *pma;
        *data++ = (uint8_t)(val & 0xFF);
        *data++ = (uint8_t)(val >> 8);
        len -= 2;
        pma += 2;
    }
    if (len == 1) {
        *data = (uint8_t)(*pma & 0xFF);
    }
}

/* ====================== EPnR 辅助 (toggle-on-write) ====================== */
static void EPnR_SetTxStat(volatile uint16_t *epnr, uint16_t stat) {
    uint16_t val = *epnr;
    val = (val & (EP_INVARIANT | EP_DTOG_TX)) ^ (stat & EPTX_STAT);
    val |= EP_CTR_RX | EP_CTR_TX;
    *epnr = val;
}

static void EPnR_SetRxStat(volatile uint16_t *epnr, uint16_t stat) {
    uint16_t val = *epnr;
    val = (val & (EP_INVARIANT | EP_DTOG_RX)) ^ (stat & EPRX_STAT);
    val |= EP_CTR_RX | EP_CTR_TX;
    *epnr = val;
}

static void EPnR_ClearCtrTx(volatile uint16_t *epnr) {
    uint16_t val = *epnr;
    val &= ~EP_CTR_TX;
    val |= EP_CTR_RX;
    *epnr = val;
}

static void EPnR_ClearCtrRx(volatile uint16_t *epnr) {
    uint16_t val = *epnr;
    val &= ~EP_CTR_RX;
    val |= EP_CTR_TX;
    *epnr = val;
}

static uint16_t PMA_Alloc(uint16_t size) {
    uint16_t off = pma_alloc_off;
    pma_alloc_off += size;
    return off;
}

static void ResetEndpoints(void) {
    uint8_t i;
    for (i = 0; i < EP_COUNT; i++) {
        memset(&ep_info[i][0], 0, sizeof(EP_Info_t));
        memset(&ep_info[i][1], 0, sizeof(EP_Info_t));
    }
}

/* ====================== ARM_DRIVER_USBD 接口 ====================== */
static ARM_DRIVER_VERSION USBD_GetVersion(void) {
    ARM_DRIVER_VERSION v = {0, 0};
    return v;
}

static ARM_USBD_CAPABILITIES USBD_GetCapabilities(void) {
    ARM_USBD_CAPABILITIES c = {0};
    return c;
}

static int32_t USBD_Initialize(ARM_USBD_SignalDeviceEvent_t   cb_dev,
                                ARM_USBD_SignalEndpointEvent_t cb_ep) {
    cb_device_event   = cb_dev;
    cb_endpoint_event = cb_ep;

    device_address   = 0;
    device_connected = 0;

    RCC->APB1ENR |= RCC_APB1Periph_USB;

    USB->CNTR   = CNTR_FRES;
    USB->BTABLE = 0;
    USB->ISTR   = 0;
    USB->CNTR   = CNTR_CTRM  | CNTR_RESETM | CNTR_SUSPM |
                  CNTR_WKUPM | CNTR_ERRM   | CNTR_PMAOVRM;

    NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 1);
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

    return ARM_DRIVER_OK;
}

static int32_t USBD_Uninitialize(void) {
    NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
    USB->CNTR = CNTR_PDWN | CNTR_FRES;
    return ARM_DRIVER_OK;
}

static int32_t USBD_PowerControl(ARM_POWER_STATE state) {
    switch (state) {
        case ARM_POWER_OFF:
            USB->CNTR = CNTR_PDWN | CNTR_FRES;
            device_address   = 0;
            device_connected = 0;
            ResetEndpoints();
            break;

        case ARM_POWER_FULL:
            USB->CNTR   = CNTR_FRES;
            USB->BTABLE = 0;
            USB->ISTR   = 0;
            USB->CNTR   = CNTR_CTRM  | CNTR_RESETM | CNTR_SUSPM |
                          CNTR_WKUPM | CNTR_ERRM   | CNTR_PMAOVRM;
            break;

        case ARM_POWER_LOW:
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }
    return ARM_DRIVER_OK;
}

static int32_t USBD_DeviceConnect(void) {
    USB->BCDR |= USB_BCDR_DPPU;
    device_connected = 1;
    return ARM_DRIVER_OK;
}

static int32_t USBD_DeviceDisconnect(void) {
    USB->BCDR &= ~USB_BCDR_DPPU;
    device_connected = 0;
    return ARM_DRIVER_OK;
}

static ARM_USBD_STATE USBD_DeviceGetState(void) {
    ARM_USBD_STATE st = {0};
    st.vbus   = device_connected;
    st.speed  = 1;
    st.active = (USB->DADDR & USB_DADDR_EF) ? 1 : 0;
    return st;
}

static int32_t USBD_DeviceRemoteWakeup(void) {
    return ARM_DRIVER_ERROR;
}

static int32_t USBD_DeviceSetAddress(uint8_t dev_addr) {
    device_address = dev_addr;
    USB->DADDR = USB_DADDR_EF | dev_addr;
    return ARM_DRIVER_OK;
}

static int32_t USBD_ReadSetupPacket(uint8_t *setup) {
    memcpy(setup, setup_buf, 8);
    EPnR_SetRxStat(&USB->EP0R, EP_RX_VALID);
    return ARM_DRIVER_OK;
}

static int32_t USBD_EndpointConfigure(uint8_t ep_addr, uint8_t ep_type, uint16_t ep_max_packet_size) {
    uint8_t ep_num = ep_addr & 0x0F;
    uint8_t dir    = (ep_addr & EP_DIR_IN) ? 1 : 0;
    volatile uint16_t *epnr;
    uint16_t pma_byte_off;
    uint16_t epr_val;

    if (ep_num >= EP_COUNT) return ARM_DRIVER_ERROR_PARAMETER;

    ep_info[ep_num][dir].max_packet_size = ep_max_packet_size;
    ep_info[ep_num][dir].configured = 1;

    switch (ep_type) {
        case ARM_USB_ENDPOINT_CONTROL:     ep_info[ep_num][dir].type = EP_T_CONTROL; break;
        case ARM_USB_ENDPOINT_ISOCHRONOUS: ep_info[ep_num][dir].type = EP_T_ISO; break;
        case ARM_USB_ENDPOINT_BULK:        ep_info[ep_num][dir].type = EP_T_BULK; break;
        case ARM_USB_ENDPOINT_INTERRUPT:   ep_info[ep_num][dir].type = EP_T_INT; break;
        default: return ARM_DRIVER_ERROR_PARAMETER;
    }

    epnr = &USB->EP0R + ep_num * 2;

    if (ep_num == 0) {
        PMA_BTABLE[0].ADDR_TX  = EP0_PMA_TX_OFF / 4;
        PMA_BTABLE[0].ADDR_RX  = EP0_PMA_RX_OFF / 4;
        PMA_BTABLE[0].COUNT_TX = 0;
        if (ep_max_packet_size > 62) {
            PMA_BTABLE[0].COUNT_RX = 0x8000 | (((ep_max_packet_size / 32) - 1) << 10);
        } else {
            PMA_BTABLE[0].COUNT_RX = ((ep_max_packet_size + 1) / 2) << 10;
        }
        epr_val = (*epnr & EP_ADDR_FIELD) | EP_T_CONTROL;
        *epnr = epr_val;
        EPnR_SetRxStat(epnr, EP_RX_VALID);
        EPnR_SetTxStat(epnr, EP_TX_NAK);
        pma_alloc_off = EP0_PMA_END;
    } else if (dir) {
        pma_byte_off = PMA_Alloc(ep_max_packet_size);
        PMA_BTABLE[ep_num].ADDR_TX  = pma_byte_off / 4;
        PMA_BTABLE[ep_num].COUNT_TX = 0;
        epr_val = (*epnr & EP_ADDR_FIELD) | ep_info[ep_num][1].type | (uint16_t)ep_num;
        *epnr = epr_val;
        EPnR_SetTxStat(epnr, EP_TX_NAK);
    } else {
        pma_byte_off = PMA_Alloc(ep_max_packet_size);
        PMA_BTABLE[ep_num].ADDR_RX = pma_byte_off / 4;
        if (ep_max_packet_size > 62) {
            PMA_BTABLE[ep_num].COUNT_RX = 0x8000 | (((ep_max_packet_size / 32) - 1) << 10);
        } else {
            PMA_BTABLE[ep_num].COUNT_RX = ((ep_max_packet_size + 1) / 2) << 10;
        }
        epr_val = (*epnr & EP_ADDR_FIELD) | ep_info[ep_num][0].type | (uint16_t)ep_num;
        *epnr = epr_val;
        EPnR_SetRxStat(epnr, EP_RX_VALID);
    }

    return ARM_DRIVER_OK;
}

static int32_t USBD_EndpointUnconfigure(uint8_t ep_addr) {
    uint8_t ep_num = ep_addr & 0x0F;
    uint8_t dir    = (ep_addr & EP_DIR_IN) ? 1 : 0;
    if (ep_num >= EP_COUNT) return ARM_DRIVER_ERROR_PARAMETER;
    ep_info[ep_num][dir].configured = 0;
    return ARM_DRIVER_OK;
}

static int32_t USBD_EndpointStall(uint8_t ep_addr, bool stall) {
    uint8_t ep_num = ep_addr & 0x0F;
    uint8_t dir    = (ep_addr & EP_DIR_IN) ? 1 : 0;
    volatile uint16_t *epnr;
    if (ep_num >= EP_COUNT) return ARM_DRIVER_ERROR_PARAMETER;
    epnr = &USB->EP0R + ep_num * 2;
    if (dir) {
        EPnR_SetTxStat(epnr, stall ? EP_TX_STALL : EP_TX_NAK);
        if (!stall) {
            uint16_t val = *epnr;
            val &= ~EP_DTOG_TX;
            val |= EP_CTR_RX | EP_CTR_TX;
            *epnr = val;
        }
    } else {
        EPnR_SetRxStat(epnr, stall ? EP_RX_STALL : EP_RX_VALID);
        if (!stall) {
            uint16_t val = *epnr;
            val &= ~EP_DTOG_RX;
            val |= EP_CTR_RX | EP_CTR_TX;
            *epnr = val;
        }
    }
    return ARM_DRIVER_OK;
}

static int32_t USBD_EndpointTransfer(uint8_t ep_addr, uint8_t *data, uint32_t num) {
    uint8_t ep_num = ep_addr & 0x0F;
    uint8_t dir    = (ep_addr & EP_DIR_IN) ? 1 : 0;
    volatile uint16_t *epnr;
    if (ep_num >= EP_COUNT) return ARM_DRIVER_ERROR_PARAMETER;

    ep_info[ep_num][dir].buf         = data;
    ep_info[ep_num][dir].num         = num;
    ep_info[ep_num][dir].transferred = 0;
    epnr = &USB->EP0R + ep_num * 2;

    if (dir) {
        uint16_t len = (num > ep_info[ep_num][1].max_packet_size) ?
                        ep_info[ep_num][1].max_packet_size : (uint16_t)num;
        PMA_Write((uint16_t)(PMA_BTABLE[ep_num].ADDR_TX * 4), data, len);
        PMA_BTABLE[ep_num].COUNT_TX = len;
        EPnR_SetTxStat(epnr, EP_TX_VALID);
    } else {
        if (ep_num == 0) {
            uint16_t val = *epnr;
            val |= EP_DTOG_RX;
            val |= EP_CTR_RX | EP_CTR_TX;
            *epnr = val;
        }
        EPnR_SetRxStat(epnr, EP_RX_VALID);
    }

    return ARM_DRIVER_OK;
}

static uint32_t USBD_EndpointTransferGetResult(uint8_t ep_addr) {
    uint8_t ep_num = ep_addr & 0x0F;
    uint8_t dir    = (ep_addr & EP_DIR_IN) ? 1 : 0;
    if (ep_num >= EP_COUNT) return 0;
    return ep_info[ep_num][dir].transferred;
}

static int32_t USBD_EndpointTransferAbort(uint8_t ep_addr) {
    uint8_t ep_num = ep_addr & 0x0F;
    uint8_t dir    = (ep_addr & EP_DIR_IN) ? 1 : 0;
    volatile uint16_t *epnr;
    if (ep_num >= EP_COUNT) return ARM_DRIVER_ERROR_PARAMETER;
    epnr = &USB->EP0R + ep_num * 2;
    if (dir) {
        EPnR_SetTxStat(epnr, EP_TX_NAK);
    } else {
        EPnR_SetRxStat(epnr, EP_RX_DIS);
    }
    return ARM_DRIVER_OK;
}

static uint16_t USBD_GetFrameNumber(void) {
    return (uint16_t)(USB->FNR & USB_FNR_FN);
}

/* ====================== 驱动实例 ====================== */
ARM_DRIVER_USBD Driver_USBD0 = {
    USBD_GetVersion,
    USBD_GetCapabilities,
    USBD_Initialize,
    USBD_Uninitialize,
    USBD_PowerControl,
    USBD_DeviceConnect,
    USBD_DeviceDisconnect,
    USBD_DeviceGetState,
    USBD_DeviceRemoteWakeup,
    USBD_DeviceSetAddress,
    USBD_ReadSetupPacket,
    USBD_EndpointConfigure,
    USBD_EndpointUnconfigure,
    USBD_EndpointStall,
    USBD_EndpointTransfer,
    USBD_EndpointTransferGetResult,
    USBD_EndpointTransferAbort,
    USBD_GetFrameNumber
};

/* ====================== USB 中断处理 ====================== */
void USB_LP_CAN1_RX0_IRQHandler(void) {
    uint16_t istr;
    uint16_t ep_id;

    while (1) {
        istr = USB->ISTR;

        if (istr & ISTR_RESET) {
            USB->ISTR = (uint16_t)~ISTR_RESET;
            USB->DADDR = USB_DADDR_EF;
            device_address = 0;
            pma_alloc_off = 0;
            ResetEndpoints();
            USB->BTABLE = 0;
            USB->CNTR = CNTR_CTRM | CNTR_RESETM | CNTR_ERRM | CNTR_PMAOVRM;
            USBD_EndpointConfigure(0, ARM_USB_ENDPOINT_CONTROL, 8);
            cb_device_event(ARM_USBD_EVENT_RESET);
            continue;
        }

        if (istr & ISTR_SUSP) {
            USB->ISTR = (uint16_t)~ISTR_SUSP;
            USB->CNTR |= CNTR_FSUSP;
            cb_device_event(ARM_USBD_EVENT_SUSPEND);
        }

        if (istr & ISTR_WKUP) {
            USB->ISTR = (uint16_t)~ISTR_WKUP;
            USB->CNTR &= ~CNTR_FSUSP;
            cb_device_event(ARM_USBD_EVENT_RESUME);
        }

        if (istr & ISTR_ERR) {
            USB->ISTR = (uint16_t)~ISTR_ERR;
        }

        if (istr & ISTR_PMAOVR) {
            USB->ISTR = (uint16_t)~ISTR_PMAOVR;
        }

        if (!(istr & ISTR_CTR)) break;

        istr = USB->ISTR;
        ep_id = istr & ISTR_EP_ID;

        if (*(volatile uint16_t *)(&USB->EP0R + ep_id * 2) & EP_CTR_TX) {
            volatile uint16_t *epnr = &USB->EP0R + ep_id * 2;
            uint32_t pkt;

            EPnR_ClearCtrTx(epnr);

            pkt = ep_info[ep_id][1].num;
            if (pkt > ep_info[ep_id][1].max_packet_size)
                pkt = ep_info[ep_id][1].max_packet_size;

            ep_info[ep_id][1].transferred += pkt;
            ep_info[ep_id][1].buf         += pkt;
            ep_info[ep_id][1].num         -= pkt;

            if (ep_info[ep_id][1].num > 0) {
                uint32_t len = ep_info[ep_id][1].num;
                if (len > ep_info[ep_id][1].max_packet_size)
                    len = ep_info[ep_id][1].max_packet_size;
                PMA_Write((uint16_t)(PMA_BTABLE[ep_id].ADDR_TX * 4),
                          ep_info[ep_id][1].buf, (uint16_t)len);
                PMA_BTABLE[ep_id].COUNT_TX = (uint16_t)len;
                EPnR_SetTxStat(epnr, EP_TX_VALID);
            } else {
                cb_endpoint_event(ep_id | EP_DIR_IN, ARM_USBD_EVENT_IN);
            }
        }

        if (*(volatile uint16_t *)(&USB->EP0R + ep_id * 2) & EP_CTR_RX) {
            volatile uint16_t *epnr = &USB->EP0R + ep_id * 2;
            uint16_t count = PMA_BTABLE[ep_id].COUNT_RX & 0x03FF;

            if (*epnr & EP_SETUP) {
                PMA_Read((uint16_t)(PMA_BTABLE[ep_id].ADDR_RX * 4), setup_buf, 8);
                EPnR_ClearCtrRx(epnr);
                cb_endpoint_event(ep_id, ARM_USBD_EVENT_SETUP);
            } else {
                if (ep_info[ep_id][0].buf && count > 0) {
                    PMA_Read((uint16_t)(PMA_BTABLE[ep_id].ADDR_RX * 4),
                             ep_info[ep_id][0].buf, count);
                    ep_info[ep_id][0].transferred += count;
                    ep_info[ep_id][0].buf         += count;
                }
                EPnR_ClearCtrRx(epnr);
                cb_endpoint_event(ep_id, ARM_USBD_EVENT_OUT);
            }
        }
    }
}
