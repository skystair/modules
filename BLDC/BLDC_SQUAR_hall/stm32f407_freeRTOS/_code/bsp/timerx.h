/**
 ******************************************************************************
 * @file    timerx.h
 * @brief   定时器/PWM 驱动层头文件
 * @details 本文件定义定时器及 PWM 输出配置
 *          - 支持10路PWM输出（8+2）
 *          - 集尘门控制（左/右）
 *          - 污水箱控制（左/右）
 *          - 风门开关、加热风扇、脏水泵等
 *          - 定时器通道映射配置
 ******************************************************************************
 */

#ifndef __timerx_h__ 
#define __timerx_h__ 

///*
//PWM_Gear_A_1（PB14 - TIM0-CHA） 
//PWM_Gear_A_2（PB15 - TIM0-CHB）

//PWM_Gear_C_1（PB08 - TIM2-CHA）
//PWM_Gear_C_2（PB09 - TIM2-CHB）

//PWM_Gear_B_1（PA08 - TIM4-CHA）
//PWM_Gear_B_2（PC09 - TIM4-CHB）

//PWM_Gear_D_2（PE13 - TIM3_CH2A）
//PWM_Gear_D_1（PE12 - TIM3_CH2B）

//PWM_FAN     （PB03 -  TIM3-CH0A）

//PWM_PUMP    （PA15 - TIM3-CH1A）
//*/
///************************ 新PWM功能宏定义（匹配最新脚位） ************************/
//// -------------------------- PWM_Gear_A_1（PB14 - TIM0-CHA） --------------------------
//#define PWM_GEAR_A1_PORT                        D_PORT_B
//#define PWM_GEAR_A1_PIN                         D_PIN_(14)
//#define PWM_GEAR_A1_AF                          D_AF_(4)  // 匹配脚位TIM0-CHA的复用功能
//#define PWM_GEAR_A1_TIMER                       TIM0     // 所属定时器
//#define PWM_GEAR_A1_CCRSEL                      Tim0CCR0A// 对应TIM0通道A
//#define PWM_GEAR_A1_dutyREG                     M0P_TIM0_MODE23->CCR0A
//// -------------------------- PWM_Gear_A_2（PB15 - TIM0-CHB） --------------------------
//#define PWM_GEAR_A2_PORT                        D_PORT_B
//#define PWM_GEAR_A2_PIN                         D_PIN_(15)
//#define PWM_GEAR_A2_AF                          D_AF_(3)  // 匹配脚位TIM0-CHB的复用功能
//#define PWM_GEAR_A2_TIMER                       TIM0     // 所属定时器
//#define PWM_GEAR_A2_CCRSEL                      Tim0CCR0B// 对应TIM0通道B
//#define PWM_GEAR_A2_dutyREG                     M0P_TIM0_MODE23->CCR0B
//// -------------------------- PWM_Gear_C_1（PB08 - -TIM2-CHA） --------------------------
//#define PWM_GEAR_C1_PORT                        D_PORT_B
//#define PWM_GEAR_C1_PIN                         D_PIN_(8)
//#define PWM_GEAR_C1_AF                          D_AF_(4)  // 匹配脚位TIM2-CHA的复用功能
//#define PWM_GEAR_C1_TIMER                       TIM2     // 所属定时器
//#define PWM_GEAR_C1_CCRSEL                      Tim2CCR0A// 对应TIM2通道A
//#define PWM_GEAR_C1_dutyREG                     M0P_TIM2_MODE23->CCR0A

//// -------------------------- PWM_Gear_C_2（PB09 - TIM2-CHB） --------------------------
//#define PWM_GEAR_C2_PORT                        D_PORT_B
//#define PWM_GEAR_C2_PIN                         D_PIN_(9)
//#define PWM_GEAR_C2_AF                          D_AF_(6)  // 匹配脚位TIM2-CHB的复用功能
//#define PWM_GEAR_C2_TIMER                       TIM2     // 所属定时器
//#define PWM_GEAR_C2_CCRSEL                      Tim2CCR0B// 对应TIM2通道B
//#define PWM_GEAR_C2_dutyREG                     M0P_TIM2_MODE23->CCR0B
//// -------------------------- PWM_Gear_B_1（PA08 - TIM4-CHA） --------------------------
//#define PWM_GEAR_B1_PORT                        D_PORT_A
//#define PWM_GEAR_B1_PIN                         D_PIN_(8)
//#define PWM_GEAR_B1_AF                          D_AF_(6)  // 匹配脚位TIM4-CHA的复用功能
//#define PWM_GEAR_B1_TIMER                       M0P_ADTIM4     // 所属定时器
//#define PWM_GEAR_B1_CCRSEL                      Tim4CCR0A// 对应TIM4通道A
//#define PWM_GEAR_B1_dutyREG                     M0P_ADTIM4->GCMAR
//// -------------------------- PWM_Gear_B_2（PC09 - TIM4-CHB） --------------------------
//#define PWM_GEAR_B2_PORT                        D_PORT_C
//#define PWM_GEAR_B2_PIN                         D_PIN_(9)
//#define PWM_GEAR_B2_AF                          D_AF_(2)  // 匹配脚位TIM4-CHB的复用功能
//#define PWM_GEAR_B2_TIMER                       TIM4     // 所属定时器
//#define PWM_GEAR_B2_CCRSEL                      Tim4CCR0B// 对应TIM4通道B
//#define PWM_GEAR_B2_dutyREG                     M0P_ADTIM4->GCMBR
//// -------------------------- PWM_Gear_D_1（PE12 - TIM3_CH2B） --------------------------
//#define PWM_GEAR_D1_PORT                        D_PORT_E
//#define PWM_GEAR_D1_PIN                         D_PIN_(12)
//#define PWM_GEAR_D1_AF                          D_AF_(1)   // 复用功能为AF1
//#define PWM_GEAR_D1_TIMER                       TIM3      // 所属定时器
//#define PWM_GEAR_D1_CCRSEL                      Tim3CCR2B // 对应TIM3通道2B
//#define PWM_GEAR_D1_dutyREG                     M0P_TIM3_MODE23->CCR2B
//// -------------------------- PWM_Gear_D_2（PE13 - TIM3_CH2A） --------------------------
//#define PWM_GEAR_D2_PORT                        D_PORT_E
//#define PWM_GEAR_D2_PIN                         D_PIN_(13)
//#define PWM_GEAR_D2_AF                          D_AF_(1)   // 复用功能为AF1
//#define PWM_GEAR_D2_TIMER                       TIM3      // 所属定时器
//#define PWM_GEAR_D2_CCRSEL                      Tim3CCR2A // 对应TIM3通道2A
//#define PWM_GEAR_D2_dutyREG                     M0P_TIM3_MODE23->CCR2A

//// -------------------------- PWM_PUMP（PA15 - TIM3-CH1A） --------------------------
//#define PWM_PUMP_PORT                           D_PORT_A
//#define PWM_PUMP_PIN                            D_PIN_(15)
//#define PWM_PUMP_AF                             D_AF_(6)  // 匹配脚位TIM3-CH1A的复用功能
//#define PWM_PUMP_TIMER                          TIM3     // 所属定时器
//#define PWM_PUMP_CCRSEL                         Tim3CCR1A// 对应TIM3通道1A（区别于CH0A/CH0B）
//#define PWM_PUMP_dutyREG                        M0P_TIM3_MODE23->CCR1A

//// -------------------------- PWM_FAN（PB03 - TIM3-CH0A） --------------------------
//#define PWM_FAN_PORT                            D_PORT_B
//#define PWM_FAN_PIN                             D_PIN_(3)
//#define PWM_FAN_AF                              D_AF_(4)  // 匹配脚位TIM3-CH0A的复用功能
//#define PWM_FAN_TIMER                           TIM3     // 所属定时器
//#define PWM_FAN_CCRSEL                          Tim3CCR0A// 对应TIM3通道0A
//#define PWM_FAN_dutyREG                         M0P_TIM3_MODE23->CCR0A

////// -------------------------- PWM_PUMP_B（PB04 - TIM3-CH0B） --------------------------
////#define PWM_PUMP_B_PORT                         D_PORT_B
////#define PWM_PUMP_B_PIN                          GpioPin4
////#define PWM_PUMP_B_AF                           D_AF_(6)  // 匹配脚位TIM3-CH0B的复用功能
////#define PWM_PUMP_B_TIMER                        TIM3     // 所属定时器
////#define PWM_PUMP_B_CCRSEL                       Tim3CCR0B// 对应TIM3通道0B


///************************ 定时器周期宏定义（匹配各PWM所属定时器） ************************/
//#define TIM0_PWM_PERIOD                         3000  // PWM_Gear_A_1/A2对应的TIM0周期 3750Hz
//#define TIM2_PWM_PERIOD                         3000  // PWM_Gear_C_1/C2对应的TIM2周期
//#define TIM3_PWM_PERIOD                         3000  // PWM_FAN/PUMP_B/PUMP对应的TIM3周期
//#define TIM4_PWM_PERIOD                         3000  // PWM_Gear_B_1/B2对应的TIM4周期

////FRpwm
//typedef struct{
//    unsigned short int targetduty;
//    unsigned short int duty;
//    volatile unsigned int* CCR;     ///< 指向CCR寄存器的指针
//    void(*dutySet)(unsigned short duty);
//}PWMstruct;


//void timerx_init(void); 
//void timerx_tick1ms(void); 
//void timerx_func(void); 


//extern PWMstruct   PWM_Str[nPWM_Max];
#endif 
