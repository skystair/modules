#ifndef __dlibx_h__
#define __dlibx_h__

//#include "dlibxConf.h"

unsigned int GetSUM(unsigned char* str,unsigned char p1,unsigned char p2);
unsigned int CalSUM_unsign(const void* addrST,unsigned char elem_size,unsigned int length);
int CalSUM_sign(const void* addrST,unsigned char elem_size,unsigned int length);

/* 配置结构已内嵌到 STRUart 中，移除独立的 dlibx_uart_cfg_t 定义 */
/* 前向声明 STRUart 以便在回调声明中使用 */
struct STRUart; /* forward */

/* 重新引入 dlibx_uart_cfg_t（配置结构），并在 STRUart 中保留 cfg 指针 */

typedef struct {
    unsigned char* txBuf;            /* 指向实例使用的 tx 缓冲区（可为 NULL，实例会使用默认） */
    unsigned char* rxBuf;            /* 指向实例使用的 rx 缓冲区 */
    const unsigned char* txHeader;   /* 发送帧头数组 */
    const unsigned char* rxHeader;   /* 接收匹配帧头数组 */
    unsigned char txHeaderLen;       /* 发送帧头长度 */
    unsigned char rxHeaderLen;       /* 接收帧头长度 */
    unsigned char rxLenInit;         /* 初始接收长度（包含头/len/chk） */
    unsigned char txLenInit;         /* 发送默认 data 长度 */
    unsigned char renAfterTx;        /* 发送完成后是否自动使能接收 */
    unsigned char txTickTrig;        /* 发送触发阈值 */

    /* 接收结果缓冲（配置部分）：rxData 指针与大小在 cfg 中定义，运行时长度写入实例 */
    unsigned char* rxData;           /* 指向用于保存提取出的 DATA 部分的缓冲区（配置） */
    unsigned char rxDataMaxLen;      /* rxData 缓冲区大小（配置） */

    /* 平台绑定回调（可由 cfg 提供以支持多实例不同硬件） */
    void (*fpTxbyte)(unsigned char data);
    void (*fpWaitTxend)(void);
    void (*fpTxINT_ENctrl)(unsigned char en);
    void (*fpRxEnCtrl)(unsigned char en);

    /* 数据组装/提取回调（无参，由用户实现根据具体实例自行访问对应实例/缓冲） */
    void (*fpTXdataSet)(void);
    void (*fpRXdataGet)(void);
} dlibx_uart_cfg_t;

typedef struct STRUart{
    // 状态与标志位
    unsigned char TXing;
    unsigned char TXcmp;
    unsigned char Rxtmp;
    unsigned char Rxcompflag;
    unsigned char RxDlenPos;
    unsigned char RxDlenplus;
    unsigned char RenAfterTx; /* runtime copy of cfg->renAfterTx */
    unsigned char Errorflag;

    // 发送/接收计数与定时
    unsigned short int TXp;
    unsigned short int TXlen;
    unsigned short int Txtick;
    unsigned short int Rxp;
    unsigned short int RxLen;
    unsigned short int RxchkLen;
    unsigned short int Rxtick;
    unsigned short int u16Errortick;

    // 缓冲区指针（运行时）
    unsigned char* Txbuff;
    unsigned char* Rxbuff;
    const unsigned char* CHKbuff;

    /* 指向配置（描述符），配置中包含头、默认缓冲等静态信息 */
    const dlibx_uart_cfg_t* cfg; /* 指向所属配置（指向 const cfg，配置可放入只读区） */

    /* 接收结果缓冲/统计（属于实例的运行时结果） */
    unsigned char* rxData;           /* 指向用于保存提取出的 DATA 部分的缓冲区 */
    unsigned char rxDataMaxLen;      /* rxData 缓冲区大小 */
    unsigned char rxDataLen;         /* 最近一帧 DATA 的长度（运行时写入） */

    // 回调函数（运行时引用，由 cfg 提供或回退）
    void (*fpTxbyte)(unsigned char data);
    void (*fpWaitTxend)(void);
    void (*fpTxINT_ENctrl)(unsigned char en);
    void (*fpRxen_ENctrl)(unsigned char en);
    void (*fpTXdataSet)(void);
    void (*fpRXdataGet)(void);
}STRUart;
#define Uart_RXIT_RXL_MAX   100
void Uart_RXIT_funcL(STRUart *Uartstruct,unsigned char pL);//变长接收,限定100
void Uart_RXIT_func(STRUart *Uartstruct);
void Uart_TXIT_func(STRUart *Uartstruct);
void uartx_Rxclear(STRUart *Uartstruct);

//1.adcFunc==========================================================
typedef struct{
    unsigned char ADnums;       //AD通道数
    unsigned char u8first;      //首次采样
    unsigned char u8cmpflag;    //完成采样

    unsigned char PWRtimes;     //2的n次方个数求和,
}ADCx_logicStruct;
typedef struct{

    unsigned short avg;
    unsigned short real;
    unsigned int u32sum;//求和窗口
}ADCx_dataStruct;

void adcx_Getvalue_func(ADCx_logicStruct* adclStr,ADCx_dataStruct* adcdStr);

//2.keyFunc==========================================================
#define KEY_FALG_SHORT      1
#define KEY_FALG_LONG       2

#define PRESS_SHORT         1
#define PRESS_LONG          2

#define KEY_VAL_PRESS		0
#define KEY_VAL_FLAG		1
#define KEY_VAL_KEEP		2
#define KEY_VAL_DELAY		3
#define KEY_VAL_RDELAY		4
typedef struct {
    unsigned char Pressing	;
    unsigned char flag 		;
    unsigned char keep 		;
    unsigned short u16tick;
    unsigned short u16Rtick;

    unsigned short Sdelay;
    unsigned short Ldelay;

    unsigned char(*pRfunc)(void);
}keyStruct;


void keyShortPressCHK(keyStruct *keys);
void keyLongPressCHK(keyStruct *keys);

void keyflagclr(keyStruct* keyx);
unsigned int keyValread(keyStruct* keyx,unsigned char valnum);

//3.ledFunc==========================================================
#define LED_STATE_OFF       0
#define LED_STATE_ON        1
#define LED_STATE_FLASH     2
//#define LED_STATE_breath    3

typedef struct{
    unsigned char onflag;

    unsigned char state;

    unsigned short Flashtick;
    unsigned short Flashondelay;
    unsigned short Flashoffdelay;

    void(*pfunc)(unsigned char flag);
}LedStruct;
void LED_statefunc(LedStruct* LEDstr);
//pwm





//norfunc
void valjudgefunc(unsigned char tf,unsigned short* tick,unsigned short delay,unsigned char* flag);
void valjudgeNcoverfunc(unsigned char tf,unsigned short* tick,unsigned short delay,unsigned char* flag);
void valJandRcoverfunc(unsigned char tf1,unsigned char tf2,unsigned short* tick,unsigned short delay,unsigned char* flag);



/*==================== 433模块实例 ====================*/
// extern STRUart uart433;
#endif
