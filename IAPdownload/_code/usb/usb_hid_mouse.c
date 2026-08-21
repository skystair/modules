#include "config.h"
#include "usb_hid_mouse.h"
#include "rl_usb.h"

/* ====================== HID Report Descriptor ====================== */
const uint8_t usbd_hid0_report_descriptor[] = {
    HID_UsagePage(HID_USAGE_PAGE_GENERIC),
    HID_Usage(HID_USAGE_GENERIC_MOUSE),
    HID_Collection(HID_Application),
        HID_Usage(HID_USAGE_GENERIC_POINTER),
        HID_Collection(HID_Physical),
            HID_UsagePage(HID_USAGE_PAGE_BUTTON),
            HID_UsageMin(1),
            HID_UsageMax(2),
            HID_LogicalMin(0),
            HID_LogicalMax(1),
            HID_ReportCount(2),
            HID_ReportSize(1),
            HID_Input(HID_Data | HID_Variable | HID_Absolute),

            HID_ReportCount(1),
            HID_ReportSize(6),
            HID_Input(HID_Constant),

            HID_UsagePage(HID_USAGE_PAGE_GENERIC),
            HID_Usage(HID_USAGE_GENERIC_X),
            HID_Usage(HID_USAGE_GENERIC_Y),
            HID_LogicalMin(0x81),
            HID_LogicalMax(0x7F),
            HID_ReportCount(2),
            HID_ReportSize(8),
            HID_Input(HID_Data | HID_Variable | HID_Relative),
        HID_EndCollection,
    HID_EndCollection
};

/* ====================== 上次报告状态 ====================== */
static uint8_t last_buttons;

/* ====================== USB HID 回调 ====================== */
void USBD_HID0_Initialize(void) {}
void USBD_HID0_Uninitialize(void) {}

int32_t USBD_HID0_GetReport(uint8_t rtype, uint8_t req, uint8_t rid, uint8_t *buf) {
    (void)rtype; (void)req; (void)rid; (void)buf;
    return 0;
}

bool USBD_HID0_SetReport(uint8_t rtype, uint8_t req, uint8_t rid, const uint8_t *buf, int32_t len) {
    (void)rtype; (void)req; (void)rid; (void)buf; (void)len;
    return false;
}

/* ====================== 初始化 ====================== */
void USB_HID_Init(void) {
    last_buttons = 0;
    USBD_Initialize(0);
    USBD_Connect(0);
}

/* ====================== 周期处理（1ms调用） ====================== */
/* 直接透传按键状态，单击/双击/长按由操作系统识别 */
void USB_HID_Mouse_Func(unsigned char btn_left, unsigned char btn_right) {
    uint8_t report[3];

    if (!USBD_Configured(0)) return;

    report[0] = (btn_left ? 1 : 0) | (btn_right ? 2 : 0);  /* bit0=left, bit1=right */
    report[1] = 0;                                           /* X = 0 */
    report[2] = 0;                                           /* Y = 0 */

    if (report[0] != last_buttons) {
        USBD_HID_GetReportTrigger(0, 0, report, 3);
        last_buttons = report[0];
    }
}
