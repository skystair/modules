/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::USB:Device
 * Copyright (c) 2004-2024 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    USBD_Config_CDC_0.h
 * Purpose: USB Device Communication Device Class (CDC) Configuration
 * Rev.:    V5.3.1
 *----------------------------------------------------------------------------*/

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//------ With VS Code: Open Preview for Configuration Wizard -------------------

// <h>USB Device: Communication Device Class (CDC) 0
//   <o>Assign Device Class to USB Device # <0-3>
//   <i>Select USB Device that is used for this Device Class instance
#define USBD_CDC0_DEV                    0

//   <o>Communication Class Subclass
//   <i>Specifies the model used by the CDC class.
//     <2=>Abstract Control Model (ACM)
//     <13=>Network Control Model (NCM)
#define USBD_CDC0_SUBCLASS               2

//   <o>Communication Class Protocol
//   <i>Specifies the protocol used by the CDC class.
//     <0=>No protocol (Virtual COM)
//     <255=>Vendor-specific (RNDIS)
#define USBD_CDC0_PROTOCOL               0

//   <h>Interrupt Endpoint Settings

//     <o.0..3>Interrupt IN Endpoint Number
//               <1=>1   <2=>2   <3=>3   <4=>4   <5=>5   <6=>6   <7=>7
//       <8=>8   <9=>9   <10=>10 <11=>11 <12=>12 <13=>13 <14=>14 <15=>15
#define USBD_CDC0_EP_INT_IN              2

//     <h>Endpoint Settings
//       <h>Full/Low-speed (High-speed disabled)
//         <o.0..6>Maximum Endpoint Packet Size (in bytes) <0-64>
#define USBD_CDC0_WMAXPACKETSIZE         16

//         <o.0..7>Endpoint polling Interval (in ms) <1-255>
#define USBD_CDC0_BINTERVAL              2

//       </h>
//       <h>High-speed
#define USBD_CDC0_HS_WMAXPACKETSIZE      16
#define USBD_CDC0_HS_BINTERVAL           2

//       </h>
//     </h>
//   </h>

//   <h>Bulk Endpoint Settings

//     <o.0..3>Bulk IN Endpoint Number
//               <1=>1   <2=>2   <3=>3   <4=>4   <5=>5   <6=>6   <7=>7
//       <8=>8   <9=>9   <10=>10 <11=>11 <12=>12 <13=>13 <14=>14 <15=>15
#define USBD_CDC0_EP_BULK_IN             1

//     <o.0..3>Bulk OUT Endpoint Number
//               <1=>1   <2=>2   <3=>3   <4=>4   <5=>5   <6=>6   <7=>7
//       <8=>8   <9=>9   <10=>10 <11=>11 <12=>12 <13=>13 <14=>14 <15=>15
#define USBD_CDC0_EP_BULK_OUT            1

//     <h>Endpoint Settings
//       <h>Full/Low-speed (High-speed disabled)
//         <o.0..6>Maximum Endpoint Packet Size (in bytes) <8=>8 <16=>16 <32=>32 <64=>64
#define USBD_CDC0_WMAXPACKETSIZE1        64

//       </h>
//       <h>High-speed
#define USBD_CDC0_HS_WMAXPACKETSIZE1     512
#define USBD_CDC0_HS_BINTERVAL1          0

//       </h>
//     </h>
//   </h>

//   <h>Communication Device Class Settings
//
//     <s.126>Communication Class Interface String
#define USBD_CDC0_CIF_STR_DESC_RAW       "USB_CDC0_0"

//     <s.126>Data Class Interface String
#define USBD_CDC0_DIF_STR_DESC_RAW       "USB_CDC0_1"

//     <h>Abstract Control Model Settings
//       <h>Call Management Capabilities
#define USBD_CDC0_ACM_CM_BM_CAPABILITIES 0x03

//       </h>
//       <h>Abstract Control Management Capabilities
#define USBD_CDC0_ACM_ACM_BM_CAPABILITIES 0x06

//       </h>
//       <o>Maximum Communication Device Send Buffer Size
//         <8=>      8 Bytes <16=>    16 Bytes <32=>    32 Bytes <64=>      64 Bytes
//         <128=>  128 Bytes <256=>  256 Bytes <512=>  512 Bytes <1024=>  1024 Bytes
//         <2048=>2048 Bytes <4096=>4096 Bytes <8192=>8192 Bytes <16384=>16384 Bytes
#define USBD_CDC0_ACM_SEND_BUF_SIZE      512

//       <o>Maximum Communication Device Receive Buffer Size
//         <8=>      8 Bytes <16=>    16 Bytes <32=>    32 Bytes <64=>      64 Bytes
//         <128=>  128 Bytes <256=>  256 Bytes <512=>  512 Bytes <1024=>  1024 Bytes
//         <2048=>2048 Bytes <4096=>4096 Bytes <8192=>8192 Bytes <16384=>16384 Bytes
#define USBD_CDC0_ACM_RECEIVE_BUF_SIZE   512

//     </h>

//     <h>Network Control Model Settings
#define USBD_CDC0_NCM_MAC_ADDRESS_RAW    "1E306CA2455E"
#define USBD_CDC0_NCM_BM_ETHERNET_STATISTICS     0x00000003
#define USBD_CDC0_NCM_W_MAX_SEGMENT_SIZE         1514
#define USBD_CDC0_NCM_W_NUMBER_MC_FILTERS        1
#define USBD_CDC0_NCM_B_NUMBER_POWER_FILTERS     0
#define USBD_CDC0_NCM_BM_NETWORK_CAPABILITIES    0x1B
#define USBD_CDC0_NCM_BM_NTB_FORMATS_SUPPORTED   0x0001
#define USBD_CDC0_NCM_DW_NTB_IN_MAX_SIZE         4096
#define USBD_CDC0_NCM_W_NDP_IN_DIVISOR           4
#define USBD_CDC0_NCM_W_NDP_IN_PAYLOAD_REMINDER  0
#define USBD_CDC0_NCM_W_NDP_IN_ALIGNMENT         4
#define USBD_CDC0_NCM_DW_NTB_OUT_MAX_SIZE        4096
#define USBD_CDC0_NCM_W_NDP_OUT_DIVISOR          4
#define USBD_CDC0_NCM_W_NDP_OUT_PAYLOAD_REMINDER 0
#define USBD_CDC0_NCM_W_NDP_OUT_ALIGNMENT        4
#define USBD_CDC0_NCM_RAW_ENABLE         0
#define USBD_CDC0_NCM_NTB_IN_BUF_CNT     1
#define USBD_CDC0_NCM_NTB_OUT_BUF_CNT    1

//     </h>
//   </h>

//   <h>OS Resources Settings
#define USBD_CDC0_INT_THREAD_STACK_SIZE  512
#define USBD_CDC0_INT_THREAD_PRIORITY    osPriorityAboveNormal
#define USBD_CDC0_BULK_THREAD_STACK_SIZE 512
#define USBD_CDC0_BULK_THREAD_PRIORITY   osPriorityAboveNormal

//   </h>
// </h>
