#ifndef __RF_433_MX_RX_h__
#define __RF_433_MX_RX_h__

/*
 * MX-05V 433MHz 接收器驱动
 *
 * 架构：EXTI 边沿中断 + TIM 定时采样
 *
 * 工作原理：
 *   1. EXTI 上升沿中断 → 同步 bit 时序（重置定时器）
 *   2. TIM 每 1ms 中断 → 在 bit 中间时刻采样 RX 引脚
 *   3. 状态机解析：前导码 → 同步字 → 长度 → 数据 → 校验和
 *
 * 接收端不再需要 tick1ms 轮询，全部由硬件中断驱动
 */

/*==================== 协议配置 ====================*/
#define RF_433_MX_RX_PREAMBLE_LEN  8       // 期望前导码字节数
#define RF_433_MX_RX_SYNC_WORD     0x2D    // 同步字
#define RF_433_MX_RX_MAX_DATA_LEN  32      // 最大数据长度

/* 超时：连续无边沿超过此值(ms)则复位到 IDLE */
#define RF_433_MX_RX_TIMEOUT_MS    20

/*==================== 状态定义 ====================*/
typedef enum {
    RF_433_MX_RX_IDLE = 0,      // 空闲，等待前导码起始边沿
    RF_433_MX_RX_PREAMBLE,      // 检测前导码
    RF_433_MX_RX_SYNC,          // 接收同步字
    RF_433_MX_RX_DATA_LEN,      // 接收数据长度
    RF_433_MX_RX_DATA,          // 接收数据
    RF_433_MX_RX_CHECKSUM,      // 接收校验和（第1次，用于校验）
    RF_433_MX_RX_CHECKSUM_SKIP, // 丢弃第2次校验和（冗余）
    RF_433_MX_RX_DONE           // 接收完成
} RF_433_MX_RX_State;

/*==================== 控制结构体 ====================*/
typedef struct {
    RF_433_MX_RX_State state;       // 当前状态
    unsigned char buffer[RF_433_MX_RX_MAX_DATA_LEN]; // 数据缓冲区
    unsigned char dataLen;          // 接收到的数据长度
    unsigned char byteIndex;        // 当前字节索引
    unsigned char bitIndex;         // 当前位索引（7→0, MSB先发）
    unsigned char preambleCount;    // 前导码字节计数
    unsigned char receivedByte;     // 正在组装的字节
    unsigned char checksum;         // 接收到的校验和
    unsigned char calcChecksum;     // 本地计算的校验和
    unsigned short timeoutCnt;      // 超时计数（ms）
    unsigned char synced;           // bit 时序已同步标志
} RF_433_MX_RX_Ctrl;

/*==================== 接口函数 ====================*/

/* 初始化 RX 模块状态机（不含硬件初始化） */
void RF_433_MX_RX_init(void);

/*
 * 处理一个 bit（由 TIM 中断在 bit 中心时刻调用）
 * @param bitValue 采样到的 bit 值（0 或 1）
 */
void RF_433_MX_RX_processBit(unsigned char bitValue);

/*
 * 外部中断回调（由 EXTI 上升沿中断调用）
 * 作用：同步 bit 时序，重置超时
 */
void RF_433_MX_RX_onRisingEdge(void);

/*
 * 超时检测（由 TIM 中断每 1ms 调用一次）
 * 无信号超过 RF_433_MX_RX_TIMEOUT_MS 则复位状态机
 */
void RF_433_MX_RX_checkTimeout(void);

/* 获取接收到的数据 */
unsigned char* RF_433_MX_RX_getData(unsigned char *len);

/* 检查接收是否完成 */
unsigned char RF_433_MX_RX_isDone(void);

/* 清除完成标志，准备下次接收 */
void RF_433_MX_RX_clearDone(void);

/*==================== 硬件抽象层 ====================*/
unsigned char RF_433_MX_RX_GetPin(void);  // 读取接收引脚电平

#endif
