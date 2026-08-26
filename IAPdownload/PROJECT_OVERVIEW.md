# IAP Downloader 工程概述

> 本文档用于新会话快速了解工程背景，包含硬件平台、软件架构、代码结构、已知问题等。

---

## 1. 硬件平台

| 项目 | 规格 |
|------|------|
| MCU | STM32F103ZET6 (512KB Flash, 64KB RAM) |
| LCD | ST7789 (ID 0x7789), 320×480 竖屏, FSMC 16位并口 |
| USB | Full-Speed 12Mbps, CDC ACM 虚拟串口 |
| 按键 | 4个: WK_UP(PA0高有效), KEY0/PE4, KEY1/PE3, KEY2/PE2 (低有效) |
| LED | DS0(PB5), DS1(PE5) — 低电平点亮 |

---

## 2. 软件架构

### 2.1 开发环境
- **IDE**: Keil MDK, ARMCLANG V6.24 (AC6)
- **RTOS**: CMSIS-RTOS RTX5 v5.9.1
- **USB中间件**: MDK-Middleware v8.3.0
- **构建命令**: `& "C:\keil\UV4\UV4.exe" -b "F:\GIT\modules\IAPdownload\stm32f103.uvprojx" -j0 -o "F:\GIT\modules\IAPdownload\build_log.txt"`

### 2.2 任务架构 (双任务)

```
┌─────────────────────────────────────────────────────┐
│  task_mode (osPriorityAboveNormal, 1ms周期)          │
│  ├─ key_func()        — 按键扫描 + 事件锁存         │
│  ├─ LEDxfunc()        — LED闪烁控制                  │
│  ├─ Board_func()      — REG_out → GPIO输出           │
│  └─ modefunc_func()   — 状态机: Prefunc→mode→Lastfunc│
│     └─ Lastfunc: USB CDC读取 → transfer_process      │
│                  + UART1透传                          │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│  task_com1 (osPriorityNormal, 10ms周期)              │
│  └─ ui_refresh()       — LCD UI刷新(500ms节流)       │
│     ├─ ui_handle_keys() — 按键事件 → 槽位选择等      │
│     ├─ 状态变化检测 → ui_refresh_status/info          │
│     └─ g_slots_dirty → ui_refresh_slots()            │
└─────────────────────────────────────────────────────┘
```

### 2.3 初始化顺序 (`main.c`)
```
GPIO时钟 → Board_Init → USB_CDC_Init → UART1_Passthrough_Init
→ tick_init → key_init → LEDxinit → lcd_init
→ flash_store_init → transfer_init → ui_init → modefunc_init
→ osKernelInitialize → task创建 → osKernelStart
```

---

## 3. 代码目录结构

```
_code/
├── config.h              — 全局头文件(包含所有模块头文件)
├── main.c                — 初始化 + 启动RTOS
│
├── bsp/                  — 板级硬件抽象
│   ├── boarddef.c/h      — GPIO引脚定义, IO_OUTch/IO_INch枚举
│   └── timerx.c/h        — 定时器配置
│
├── device/               — 设备驱动
│   ├── key.c/h           — 按键驱动(扫描+消抖+事件锁存)
│   ├── led.c/h           — LED驱动
│   └── lcd/
│       ├── lcd.c         — LCD驱动(ST7789/ILI9341等, FSMC接口)
│       ├── lcd.h         — LCD API(lcd_init, lcd_clear, lcd_show_string等)
│       ├── lcd_ex.c      — LCD寄存器初始化扩展
│       └── lcd_display.c/h — 底层显示函数
│
├── func/                 — 功能模块
│   ├── modefunc.c/h      — 状态机(modech_idle/func1/func2)
│   ├── ui_display.c      — IAP Downloader UI (40列×30行, 深色主题)
│   └── (lcd_show_char使用g_back_color绘制字符背景)
│
├── storage/              — 存储模块
│   ├── flash_store.c/h   — Flash文件系统(索引区+5×84KB槽位)
│   ├── xmodem.c/h        — XMODEM-128/CRC协议(状态机)
│   └── transfer.c/h      — USB传输控制(命令解析+XMODEM接收+Flash写入)
│
├── usb/                  — USB CDC驱动
│   ├── usb_cdc.c         — USB CDC ACM实现(USBD回调+收发API)
│   ├── usb_cdc_app.h     — USB CDC API(Init/Send/Receive/DataAvailable)
│   ├── usb_hid_mouse.c/h — HID鼠标(备用)
│   └── USBD_STM32F10x.c/h — USB设备底层驱动
│
├── task/                 — RTOS任务
│   ├── task_mode.c/h     — task_mode: 1ms周期(按键/LED/状态机)
│   └── task_com1.c/h     — task_com1: 10ms周期(UI刷新)
│
└── lib/                  — 通用库
    ├── dlibx.c/h         — 库函数封装
    ├── dlibx_key.c/h     — 按键库
    ├── dlibx_led.c/h     — LED库
    └── dlibx_uart.c/h    — UART库
```

---

## 4. 核心模块详解

### 4.1 Flash文件系统 (`flash_store.c/h`)

**Flash布局** (STM32F103ZET6, 512KB):
```
0x08000000 ┌──────────────┐  72KB  程序区
0x08012000 ├──────────────┤  16KB  索引区(8页×2KB)
0x08016000 ├──────────────┤
           │  Slot 0 84KB │  数据槽位
           │  Slot 1 84KB │
           │  Slot 2 84KB │
           │  Slot 3 84KB │
           │  Slot 4 84KB │
0x0807F000 └──────────────┘
```

**索引结构**: `file_index_t` = 16B头 + 5×32B条目 = 176字节
- 每条目: state(4B) + start_addr(4B) + size(4B) + crc32(4B) + name(24B)
- 状态: FILE_STATE_EMPTY(0xFFFFFFFF) / FILE_STATE_VALID("VALD") / FILE_STATE_DELETED(0x00000000)

**流式写入优化**: `flash_store_write_stream_begin/end` — 整个传输期间Flash保持Unlock，避免每128字节Unlock/Lock

### 4.2 XMODEM协议 (`xmodem.c/h`)

**协议**: XMODEM-128/CRC, 兼容小米IoT MCU OTA
**包格式**: `SOH(1) | PKT#(1) | ~PKT#(1) | DATA(128) | CRC_H(1) | CRC_L(1)` = 133字节
**状态机**: STATE_INIT → WAIT_SOH → RECV_PKT → RECV_NPKT → RECV_DATA → RECV_CRC_H → RECV_CRC_L → VERIFY → DONE
**超时**: XMODEM_CHAR_TIMEOUT=1000ms, XMODEM_TIMEOUT_MS=10000ms(首包)

### 4.3 USB传输控制 (`transfer.c/h`)

**命令协议** (ASCII, 以`\n`结尾):
| 命令 | 格式 | 说明 |
|------|------|------|
| Q | `Q\n` | 查询文件列表 |
| S | `S0\n`~`S4\n` | 选择槽位 |
| T | `T\n` 或 `T:filename\n` | 开始XMODEM传输 |
| D | `D0\n`~`D4\n` | 删除文件 |
| E | `E\n` | 擦除槽位 |
| V | `V\n` | 验证文件 |
| I | `I\n` | 获取MCU信息 |

**响应格式**: `K:消息\n`(成功) / `E:消息\n`(错误) / `R:消息\n`(就绪)

**传输状态**: IDLE → SELECTED → RECEIVING → DONE/ERROR / DELETED

### 4.4 UI显示 (`ui_display.c`)

**LCD**: 320×480竖屏, 8×16字体, 40列×30行
**主题**: 深色背景(CLR_BG=0x0010)
**刷新**: task_com1每10ms调用ui_refresh(), 内部500ms节流
**关键全局变量**: `g_back_color`(lcd.c中定义) 控制字符背景色, UI绘制前需设为CLR_ROW_BG

**布局**(30行):
```
Row 0:  标题 "==== IAP Downloader v1.0 ===="
Row 1:  信息 "USB:OK   Slots:2/5   FW:v1.0"
Row 2:  分隔线
Row 3:  表头 " #  File Name              Size  CRC"
Row 4-8: 槽位 S0~S4 (选中项显示黄色>前缀)
Row 9:  分隔线
Row 10: 状态 "Status: Idle/Receiving/Done"
Row 11: 进度条
Row 12: 传输数据
Row 13: CRC校验
Row 14-18: 命令区
Row 20: 消息反馈
Row 22-24: 远程/槽位/文件操作
Row 26-28: 传输详情(进度/文件名/速度)
Row 29: 版本信息
```

### 4.5 按键系统 (`key.c`)

**扫描**: key_func() 1ms周期调用, 通过keyShortPressCHK()消抖
**事件锁存**: g_key_event[]数组, key_func末尾捕获flag
**UI读取**: key_event_read(ch) — 读后清除, 配合10ms UI周期不漏检
**按键映射**: UP→槽位上移, DOWN→槽位下移, LEFT/RIGHT→备用

---

## 5. 上位机工具

- **技术栈**: Visual Studio, .NET Framework 4.7.2
- **功能**: USB CDC串口通信, XMODEM文件发送, 槽位管理
- **PC工具文件**: 位于外部(非本仓库), 协议变更时需MCU+PC同步更新

---

## 6. 已完成的优化与修复

### 6.1 LCD白色背景修复
- **问题**: lcd.c中 `g_back_color = 0xFFFF`(白色), lcd_show_char()用此色绘制字符背景
- **修复**: ui_display.c中 lcd_show_string 前设置 `g_back_color = CLR_ROW_BG`

### 6.2 USB传输速度优化
- **原瓶颈**: xmodem_recv_byte()逐字节读取 + osDelay(1) = ~0.8 KB/s
- **优化1**: 批量缓冲读取 — g_xm_recv_buf[133], 一次USB_CDC_Receive读取所有可用数据
- **优化2**: Flash流式模式 — flash_store_write_stream_begin/end, 整个传输仅1次Unlock/Lock
- **优化3**: XMODEM_CHAR_TIMEOUT 从3000ms降为1000ms
- **注意**: osDelay(0)会导致紧密轮询占满CPU, USB中断无法及时处理, 引发重试。必须保留osDelay(1)

### 6.3 删除后UI刷新
- **问题**: handle_delete()删除文件后未通知UI刷新槽位列表
- **修复**: 添加TRANSFER_DELETED状态, handle_delete成功后set_state(TRANSFER_DELETED), UI回调中设置g_slots_dirty=1

---

## 7. 构建注意事项

### 7.1 编译缓存问题
Keil不会自动检测头文件变更。修改.h文件后必须:
1. 删除对应的.o文件
2. 用`Get-Content build_log.txt | Select-String "compiling"`确认文件被重新编译

### 7.2 标准编译流程
```powershell
# 删除受影响的.o文件
Remove-Item "F:\GIT\modules\IAPdownload\Objects\xxx.o" -ErrorAction SilentlyContinue
# 编译
& "C:\keil\UV4\UV4.exe" -b "F:\GIT\modules\IAPdownload\stm32f103.uvprojx" -j0 -o "F:\GIT\modules\IAPdownload\build_log.txt"
# 检查结果
Get-Content "F:\GIT\modules\IAPdownload\build_log.txt" | Select-String -Pattern "compiling|Error|0 Error" | Select-Object -Last 5
```

---

## 8. 关键API速查

| 模块 | API | 说明 |
|------|-----|------|
| USB CDC | `USB_CDC_Send(data, len)` | 发送数据到PC |
| USB CDC | `USB_CDC_Receive(buf, max)` | 接收数据(支持批量) |
| USB CDC | `USB_CDC_DataAvailable()` | 查询接收缓冲区 |
| Flash | `flash_store_init()` | 初始化存储系统 |
| Flash | `flash_store_write_begin(slot, name)` | 开始流式写入(擦除) |
| Flash | `flash_store_write_data(offset, data, len)` | 写入数据块 |
| Flash | `flash_store_write_end(size, crc32)` | 结束写入(更新索引) |
| Flash | `flash_store_delete(idx)` | 删除文件 |
| Flash | `flash_store_get_info(idx, info)` | 获取文件信息 |
| Flash | `flash_store_write_stream_begin/end()` | 流式模式(批量Unlock) |
| Transfer | `transfer_process(data, len)` | 处理USB接收数据 |
| Transfer | `transfer_get_state()` | 获取传输状态 |
| Transfer | `transfer_set_callback(cb)` | 注册状态回调 |
| UI | `ui_init()` | 初始化UI(全屏绘制) |
| UI | `ui_refresh()` | 刷新UI(500ms节流) |
| UI | `ui_full_redraw()` | 强制全屏重绘 |
| LCD | `lcd_show_string(x, y, str, color)` | 显示字符串 |
| LCD | `lcd_clear(color)` | 清屏 |
| LCD | `g_back_color` | 字符背景色(需在显示前设置) |
| Key | `key_event_read(ch)` | 读取按键事件(读后清除) |
| Key | `key_state_read(ch)` | 读取按键物理状态 |
