# STM32F103 USB HID 鼠标工程代码说明

## 一、工程概述

本工程基于 STM32F103C8T6（Blue Pill）核心板，实现 USB HID 鼠标设备：
- **PC13 按键** → 鼠标左键
- **PA0 按键** → 鼠标右键
- 使用 RTX5 RTOS 多任务架构
- 使用 Keil MDK-Middleware USB 组件 + CMSIS USBD Driver

---

## 二、目录结构

```
_code/
├── main.c              # 主程序入口
├── config.h            # 全局头文件汇总
├── bsp/
│   ├── boarddef.h      # 板级硬件定义（GPIO枚举、结构体）
│   └── boarddef.c      # GPIO配置、输入输出刷新
├── device/
│   ├── key.h/c         # 按键驱动（消抖、短按长按检测）
│   └── led.h/c         # LED驱动
├── lib/
│   ├── dlibx.h         # 通用库（按键状态机、LED控制）
│   └── dlibxConf.h     # 按键/LED通道配置
├── func/
│   └── modefunc.c      # 状态机调度、业务逻辑
├── task/
│   ├── task_mode.c     # 主任务（1ms周期）
│   └── task_com1.c     # 通信任务
└── usb/
    ├── usb_hid_mouse.h/c       # HID鼠标应用层
    ├── USBD_STM32F10x.h/c      # CMSIS USB Device 驱动
    └── GPIO_STM32F10x.h/c      # GPIO辅助驱动
RTE/
├── USB/
│   ├── USBD_Config_0.h         # USB设备0配置（VID/PID/端点0）
│   └── USBD_Config_HID_0.h    # HID类配置（报告大小、端点）
└── Device/STM32F103C8/
    └── RTE_Device.h            # 设备引脚配置（D+上拉）
```

---

## 三、系统初始化流程

```c
// main.c
int main(void){
    main_init();           // 硬件初始化
    osKernelInitialize();  // RTOS内核初始化
    USB_HID_Init();        // USB HID初始化
    task_mode_creat();     // 创建主任务
    task_com1_creat();     // 创建通信任务
    osKernelStart();       // 启动RTOS调度
}
```

**main_init() 顺序：**
1. 使能 GPIOA/GPIOC/TIM3 时钟
2. `Board_Init()` → 配置所有GPIO引脚
3. `tick_init()` → 定时器初始化
4. `key_init()` → 注册按键读取函数
5. `LEDxinit()` → LED初始化
6. `modefunc_init()` → 状态机初始化

---

## 四、任务架构

```c
// task_mode.c - 1ms周期任务
void task_mode(void *pvParameters){
    while(1){
        key_func();         // 按键扫描（消抖、状态检测）
        LEDxfunc();         // LED闪烁控制
        Board_func();       // GPIO刷新（REG_out→引脚, 引脚→REG_in）
        modefunc_func();    // 业务逻辑调度
        osDelayUntil(tick); // 1ms精确延时
    }
}
```

---

## 五、按键模块

### 5.1 硬件配置（boarddef.h/c）

```c
// 输入通道枚举
typedef enum {
    IO_INch_KEY_USR = 0,     // PC13 - 用户按键
    IO_INch_KEY_WAKEUP,      // PA0  - Wakeup按键
    IO_INch_Max
} io_in_ch_t;

// GPIO配置数组
const BoardGpioConfig_t Board_GpioInputs[IO_INch_Max] = {
    {D_PORT_C, D_PIN_(13), 0},  // PC13，上拉输入，按下=0
    {D_PORT_A, D_PIN_(0),  0},  // PA0，下拉输入，按下=1
};
```

### 5.2 按键读取（key.c）

```c
// 注册读取函数
void key_init(void){
    keystr[IO_INch_KEY_USR].pRfunc    = &keyRead_test;    // PC13
    keystr[IO_INch_KEY_WAKEUP].pRfunc = &keyRead_wakeup;  // PA0
}

// 读取函数返回 Board_REG_in[] 的值
unsigned char keyRead_test(void){
    return Board_REG_in[IO_INch_KEY_USR];
}
```

### 5.3 按键状态机（dlibx库）

按键状态机提供：
- `KEY_VAL_PRESS` - 当前是否按下（消抖后）
- `KEY_VAL_FLAG` - 按下事件标志（上升沿）
- 短按/长按检测（50ms短按，1500ms长按）

---

## 六、USB HID 模块详解 ⭐

### 6.1 USB 协议栈架构

```
┌─────────────────────────────────────────────────────────────┐
│                    应用层 (usb_hid_mouse.c)                  │
│         USB_HID_Mouse_Func() → 构造报告 → 发送              │
├─────────────────────────────────────────────────────────────┤
│              Keil MDK-Middleware USB (rl_usb.h)              │
│         USBD_HID_GetReportTrigger() → HID类处理             │
├─────────────────────────────────────────────────────────────┤
│           CMSIS USBD Driver (USBD_STM32F10x.c)              │
│         ARM_DRIVER_USBD 接口 → 寄存器操作                    │
├─────────────────────────────────────────────────────────────┤
│                STM32F103 USB 外设硬件                        │
│         USB寄存器 (0x40005C00) + PMA (0x40006000)           │
└─────────────────────────────────────────────────────────────┘
```

### 6.2 CMSIS USBD Driver（USBD_STM32F10x.c/h）

这是 USB 底层驱动，直接操作 STM32F103 的 USB 寄存器。

#### 6.2.1 寄存器定义（USBD_STM32F10x.h）

```c
#define USB_BASE_ADDR   0x40005C00  // USB寄存器基地址
#define USB_PMA_ADDR    0x40006000  // Packet Memory Area地址

// 主要寄存器
#define CNTR    REG(USB_BASE_ADDR + 0x40)   // 控制寄存器
#define ISTR    REG(USB_BASE_ADDR + 0x44)   // 中断状态寄存器
#define FNR     REG(USB_BASE_ADDR + 0x48)   // 帧号寄存器
#define DADDR   REG(USB_BASE_ADDR + 0x4C)   // 设备地址寄存器
#define BTABLE  REG(USB_BASE_ADDR + 0x50)   // 缓冲区表地址

// D+上拉控制（Blue Pill必须使用内部上拉）
#define BCDR        REG(USB_BASE_ADDR + 0x58)
#define BCDR_DPPU   0x8000  // DP Pull-Up Enable
```

#### 6.2.2 关键修改点

**1. D+上拉控制（Blue Pill 特有）**

Blue Pill 没有外部 D+ 上拉电阻，必须使用 STM32 内部上拉：

```c
// USBD_STM32F10x.c - DeviceConnect
static int32_t USBD_DeviceConnect(void) {
    CNTR = CNTR_FRES;                    // 复位USB
    CNTR = CNTR_CTRM | CNTR_RESETM;     // 使能中断
    ISTR = 0;                            // 清中断标志
    DADDR = DADDR_EF;                    // 使能USB功能
    BCDR |= BCDR_DPPU;                   // ← 使能D+内部上拉
    return ARM_DRIVER_OK;
}

// DeviceDisconnect
static int32_t USBD_DeviceDisconnect(void) {
    BCDR &= ~BCDR_DPPU;                 // ← 禁用D+上拉
    CNTR = CNTR_FRES | CNTR_PDWN;       // 断开USB
    return ARM_DRIVER_OK;
}
```

**2. PMA 缓冲区操作**

STM32F103 的 USB 外设有专用的 Packet Memory Area (PMA)，与主内存不连续，需要特殊读写：

```c
// 从PMA读取数据（PMA是32位对齐，但数据是16位）
ptr_src = (uint32_t *)(USB_PMA_ADDR + 2*((pBUF_DSCR + ep_num)->ADDR_RX));

// 写入PMA
ptr_dest = (uint32_t *)(USB_PMA_ADDR + 2*((pBUF_DSCR + ep_num)->ADDR_TX));
*ptr_dest++ = *ptr_src++;
```

**3. 中断处理（USB_IRQHandler）**

```c
void USB_IRQHandler(void) {
    // 1. 处理正确的传输完成（CTR）
    while(1) {
        val = ISTR;
        if (!(val & ISTR_CTR)) break;
        // 处理端点数据收发...
    }

    // 2. 处理USB复位
    if (val & ISTR_RESET) {
        // 初始化端点0...
    }

    // 3. 处理挂起/唤醒
    if (val & ISTR_SUSP) { ... }
    if (val & ISTR_WKUP) { ... }
}
```

### 6.3 HID 类配置（USBD_Config_HID_0.h）

```c
// 端点配置
#define USBD_HID0_EP_INT_IN                 1      // IN端点号
#define USBD_HID0_EP_INT_IN_WMAXPACKETSIZE  4      // 最大包大小
#define USBD_HID0_EP_INT_IN_BINTERVAL       16     // 轮询间隔(ms)

// HID报告配置
#define USBD_HID0_IN_REPORT_MAX_SZ          3      // 输入报告大小（字节）
#define USBD_HID0_USER_REPORT_DESCRIPTOR    1      // 使用自定义描述符
#define USBD_HID0_USER_REPORT_DESCRIPTOR_SIZE 50   // 描述符大小
```

### 6.4 HID Report Descriptor（usb_hid_mouse.c）

这是 HID 设备的核心，定义了设备向主机报告数据的格式。

```c
const uint8_t usbd_hid0_report_descriptor[] = {
    // 鼠标设备
    HID_UsagePage(HID_USAGE_PAGE_GENERIC),      // 通用桌面页
    HID_Usage(HID_USAGE_GENERIC_MOUSE),          // 鼠标
    HID_Collection(HID_Application),             // 应用集合
        HID_Usage(HID_USAGE_GENERIC_POINTER),    // 指针
        HID_Collection(HID_Physical),            // 物理集合

            // ===== 按钮部分 =====
            HID_UsagePage(HID_USAGE_PAGE_BUTTON), // 按钮页
            HID_UsageMin(1),                       // 按钮1
            HID_UsageMax(2),                       // 到按钮2（左键+右键）
            HID_LogicalMin(0),                     // 值范围: 0
            HID_LogicalMax(1),                     // 到 1
            HID_ReportCount(2),                    // 2个按钮
            HID_ReportSize(1),                     // 每个1位
            HID_Input(HID_Data | HID_Variable | HID_Absolute),

            // 填充位（对齐到字节）
            HID_ReportCount(1),
            HID_ReportSize(6),                     // 6位填充
            HID_Input(HID_Constant),

            // ===== X/Y轴部分 =====
            HID_UsagePage(HID_USAGE_PAGE_GENERIC),
            HID_Usage(HID_USAGE_GENERIC_X),        // X轴
            HID_Usage(HID_USAGE_GENERIC_Y),        // Y轴
            HID_LogicalMin(0x81),                  // -127（有符号）
            HID_LogicalMax(0x7F),                  // +127
            HID_ReportCount(2),                    // X和Y
            HID_ReportSize(8),                     // 每个8位
            HID_Input(HID_Data | HID_Variable | HID_Relative),

        HID_EndCollection,
    HID_EndCollection
};
```

**报告格式（3字节）：**
```
Byte 0: [bit1:右键] [bit0:左键] [bit7-2:填充]
Byte 1: X轴移动（有符号，-127~+127）
Byte 2: Y轴移动（有符号，-127~+127）
```

### 6.5 鼠标应用层（usb_hid_mouse.c）

```c
// 上次报告状态（用于变化检测）
static uint8_t last_buttons;

// 初始化
void USB_HID_Init(void) {
    last_buttons = 0;
    USBD_Initialize(0);   // 初始化USB设备0
    USBD_Connect(0);      // 连接USB（使能D+上拉）
}

// 周期调用（由modefunc_Lastfunc调用）
void USB_HID_Mouse_Func(unsigned char btn_left, unsigned char btn_right) {
    uint8_t report[3];

    if (!USBD_Configured(0)) return;  // 未配置则返回

    // 构造报告
    report[0] = (btn_left ? 1 : 0) | (btn_right ? 2 : 0);  // 按钮状态
    report[1] = 0;                                           // X = 0（不移动）
    report[2] = 0;                                           // Y = 0（不移动）

    // 状态变化时才发送（避免重复发送）
    if (report[0] != last_buttons) {
        USBD_HID_GetReportTrigger(0, 0, report, 3);  // 发送报告
        last_buttons = report[0];                     // 更新状态
    }
}
```

### 6.6 HID 回调函数

```c
// 设备初始化/反初始化回调
void USBD_HID0_Initialize(void) {}
void USBD_HID0_Uninitialize(void) {}

// GET_REPORT 请求处理（主机主动读取）
int32_t USBD_HID0_GetReport(uint8_t rtype, uint8_t req, uint8_t rid, uint8_t *buf) {
    return 0;  // 使用默认处理
}

// SET_REPORT 请求处理（主机发送数据）
bool USBD_HID0_SetReport(uint8_t rtype, uint8_t req, uint8_t rid,
                         const uint8_t *buf, int32_t len) {
    return false;  // 不支持
}
```

---

## 七、配置文件说明

### 7.1 RTE_Device.h

```c
#define RTE_USB_DEVICE_CON_PIN  0  // 禁用外部D+上拉引脚
                                   // Blue Pill使用BCDR.DPPU内部上拉
```

### 7.2 USBD_Config_0.h

```c
#define USBD0_DEV_DESC_IDVENDOR   0xC251   // Vendor ID (Keil测试用)
#define USBD0_DEV_DESC_IDPRODUCT  0x0000   // Product ID
#define USBD0_MAX_PACKET0         8        // 端点0最大包大小
#define USBD0_CFG_DESC_BMAXPOWER  250      // 最大电流125mA
```

---

## 八、数据流示意

```
按键按下
   ↓
Board_func() → GPIO_ReadInputDataBit() → Board_REG_in[通道]
   ↓
key_func() → 消抖处理 → keystr[].press 状态
   ↓
modefunc_Lastfunc() → keyXvalread(通道, KEY_VAL_PRESS)
   ↓
USB_HID_Mouse_Func(btn_left, btn_right)
   ↓
构造 report[3] = {按钮状态, X, Y}
   ↓
USBD_HID_GetReportTrigger() → HID类处理
   ↓
USB_IRQHandler() → 端点1 IN传输
   ↓
主机接收HID报告 → 驱动鼠标动作
```

---

## 九、开发过程中遇到的关键问题

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| USB无法枚举 | Blue Pill无外部D+上拉 | 添加 BCDR.DPPU 控制 |
| `__packed` 报错 | armclang不支持指针上的packed | 移除指针上的`__packed` |
| `osDelay`崩溃 | 内核启动前调用RTOS函数 | 改为忙等待 |
| 鼠标无反应 | Report大小配置为1字节 | 改为3字节匹配描述符 |
| Report描述符大小不匹配 | 配置值与实际不符 | 修正为实际50字节 |

---

## 十、扩展建议

如需添加更多功能：

1. **鼠标移动** - 修改 report[1] (X) 和 report[2] (Y)
2. **滚轮** - 在描述符中添加 HID_USAGE_GENERIC_WHEEL
3. **多媒体键** - 添加 HID Consumer Control 设备
4. **键盘** - 添加 HID Keyboard 设备（需新端点和描述符）

---

*文档生成时间: 2026-07-31*
*工程路径: e:\sources\modules\USB_hid*
