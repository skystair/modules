#ifndef __RF_433_MX_TX_h__
#define __RF_433_MX_TX_h__

/* 发射器配置 */
#define RF_433_MX_TX_PREAMBLE_LEN  8       // 前导码长度（字节）
#define RF_433_MX_TX_SYNC_WORD     0x2D    // 同步字
#define RF_433_MX_TX_MAX_DATA_LEN  32      // 最大数据长度

/* 发射器状态定义 */
typedef enum {
    RF_433_MX_TX_IDLE = 0,      // 空闲
    RF_433_MX_TX_PREAMBLE,      // 发送前导码
    RF_433_MX_TX_SYNC,          // 发送同步字
    RF_433_MX_TX_DATA_LEN,      // 发送数据长度
    RF_433_MX_TX_DATA,          // 发送数据
    RF_433_MX_TX_CHECKSUM,      // 发送校验和（第1次）
    RF_433_MX_TX_CHECKSUM2,     // 发送校验和（第2次，冗余）
    RF_433_MX_TX_DONE           // 发送完成
} RF_433_MX_TX_State;

/* 发射器控制结构体 */
typedef struct {
    RF_433_MX_TX_State state;       // 当前状态
    unsigned char dataLen;          // 数据长度
    unsigned char byteIndex;        // 当前字节索引
    unsigned char bitIndex;         // 当前位索引（7-0）
    unsigned char preambleCount;    // 前导码计数器
    unsigned char checksum;         // 校验和
    unsigned char *data;            // 待发送数据指针
} RF_433_MX_TX_Ctrl;

/* 函数声明 */
void RF_433_MX_TX_init(void);
void RF_433_MX_TX_tick1ms(void);
void RF_433_MX_TX_func(void);

/* 发射数据函数：data为数据指针，len为数据长度（0~32） */
unsigned char RF_433_MX_TX_send(unsigned char *data, unsigned char len);

/* 查询发射器是否空闲（可接受新数据） */
unsigned char RF_433_MX_TX_isIdle(void);

/* 硬件抽象层函数（需要在具体平台实现） */
void RF_433_MX_TX_SetPin(unsigned char level);  // 设置发射引脚电平
void RF_433_MX_TX_DelayUs(unsigned short us);   // 微秒延时（可选）

#endif
