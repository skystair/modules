/**
 ******************************************************************************
 * @file    dlibx.h
 * @brief   驱动库功能头文件
 * @details 本文件定义通用的驱动层数据结构及工具函数
 *          - UART 通信结构体（发送/接收缓冲区、状态标志）
 *          - 校验和计算函数
 *          - 通用数据处理工具
 *          - 设备状态管理结构
 ******************************************************************************
 */

#ifndef __dlibx_h__ 
#define __dlibx_h__ 

//#include "dlibxConf.h"

unsigned int GetSUM(unsigned char* str,unsigned char p1,unsigned char p2);
//0.uartxFunc==========================================================
typedef struct{    
    unsigned char TXing;
    unsigned char TXcmp;
    unsigned char* Txbuff;
    unsigned short int TXp;
    unsigned short int TXlen;
    unsigned short int Txtick;//tx on
    
    unsigned char Rxtmp;
    unsigned char Rxcompflag;
    unsigned char RxDlenPos;
    unsigned char* CHKbuff;
//    unsigned char* CHKpos;
    unsigned char* Rxbuff;
    unsigned short int Rxp;
    unsigned short int RxLen;
    unsigned short int RxchkLen;
    unsigned short int Rxtick;
    
    unsigned char RenAfterTx;
    unsigned char       Errorflag;
    unsigned short int  u16Errortick;
    //funcp
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
    
//    unsigned char(*pRfunc)(void);
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
    unsigned char* onflag;
    
    unsigned char state;
    
    unsigned short Flashtick;
    unsigned short Flashondelay;
    unsigned short Flashoffdelay;
    
//    void(*pfunc)(unsigned char flag);
}LedStruct;
void LED_statefunc(LedStruct* LEDstr);
//pwm





//norfunc
void valjudgefunc(unsigned char tf,unsigned short* tick,unsigned short delay,unsigned char* flag);
void valjudgeNcoverfunc(unsigned char tf,unsigned short* tick,unsigned short delay,unsigned char* flag);
void valJandRcoverfunc(unsigned char tf1,unsigned char tf2,unsigned short* tick,unsigned short delay,unsigned char* flag);

#endif

