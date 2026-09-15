/**
 ******************************************************************************
 * @file    BLDC_hall.h
 * @brief   BLDC 霍尔传感器读取与处理
 ******************************************************************************
 */

#ifndef __bldc_hall_h__
#define __bldc_hall_h__

/* Hall 状态结构体 */
typedef struct {
    unsigned char state;          /* 当前 Hall 状态 (0-7) */
    unsigned char state_prev;     /* 上一次 Hall 状态 */
    unsigned char u;              /* Hall U 相状态 */
    unsigned char v;              /* Hall V 相状态 */
    unsigned char w;              /* Hall W 相状态 */
    unsigned char sector;         /* 当前扇区 (1-6) */
} BLDC_Hall_t;

/* 全局变量声明 */
extern BLDC_Hall_t g_hall;

/* 函数声明 */
void BLDC_Hall_Init(void);
void BLDC_Hall_Read(void);
unsigned char BLDC_Hall_GetSector(unsigned char hall_state);

#endif /* __bldc_hall_h__ */
