#include "timerx.h" 
//PWMstruct   PWM_Str[nPWM_Max];

//void timerx_init(void); 
//void timerx_tick1ms(void); 
//void timerx_func(void); 

///**
// * @brief Timer/PWM IO初始化（基于board配置）
// * @note  新版本：使用board_v1.h中的配置数组自动初始化引脚
// */
//void timerx_IO_init(void){
//    stc_gpio_cfg_t stcGpioCfg;
//    DDL_ZERO_STRUCT(stcGpioCfg);

//    stcGpioCfg.enDir = GpioDirOut;
//    stcGpioCfg.enDrv = GpioDrvH;
//    stcGpioCfg.enOD = GpioOdDisable;
//    
//    // 遍历所有PWM通道，根据board配置初始化引脚
//    for(uint8_t i = 0; i < nPWM_Max; i++) {
//        Gpio_Init(Board_Pwms[i].GPIO.port, Board_Pwms[i].GPIO.pin, &stcGpioCfg);
//        Gpio_SetAfMode(Board_Pwms[i].GPIO.port, Board_Pwms[i].GPIO.pin, Board_Pwms[i].GPIO.Af);
//        PWM_Str[i].CCR = Board_Pwms[i].CCR;
//    }
//}
//#if 1//tim012
///*
//*******************************************************************************
//----Function Description              : PWM_Gear_A组（A1=PB14-TIM0-CHA、A2=PB15-TIM0-CHB）PWM初始化
//----Function Name                      : PWM_GearA_Init(void)
//----Input                               : None
//----Output                              : None
//----Return                              : None
//----Note                                : 共用Base Timer(TIM0)，两通道独立PWM输出
//                                        频率：≈3.75kHz（48M/8/1600=3750Hz）
//                                        周期同步，占空比独立可调（通过各自SetDuty函数）
//*******************************************************************************
//*/
//void PWM_GearA_Init(void){
//    stc_bt_mode23_cfg_t        stcBtBaseCfg;        // TIM0模式23基础配置结构体
//    stc_bt_m23_compare_cfg_t   stcBtPortCmpCfg;     // TIM0通道比较输出配置结构体

//    DDL_ZERO_STRUCT(stcBtBaseCfg);
//    DDL_ZERO_STRUCT(stcBtPortCmpCfg);

//    // 4. 配置TIM0模式23基础参数（两通道共用，仅配置一次）
//    stcBtBaseCfg.enWorkMode    = BtWorkMode2;              // 工作模式：锯齿波模式
//    stcBtBaseCfg.enCT          = BtTimer;                  // 功能选择：定时器模式（内部PCLK）
//    stcBtBaseCfg.enPRS         = BtPCLKDiv16;               // 预分频系数：8（计数时钟=6M）
//    stcBtBaseCfg.enCntDir      = BtCntUp;                  // 计数方向：向上计  数
//    stcBtBaseCfg.enPWMTypeSel  = BtIndependentPWM;         // PWM类型：独立输出（两通道互不影响）
//    stcBtBaseCfg.enPWM2sSel    = BtSinglePointCmp;         // 比较模式：单点比较（各用自身CCR）
//    stcBtBaseCfg.bOneShot      = FALSE;                    // 计数模式：循环计数
//    stcBtBaseCfg.bURSSel       = FALSE;                    // 更新源：上下溢均更新
//    Bt_Mode23_Init(PWM_GEAR_A1_TIMER, &stcBtBaseCfg);       

//    // 5. 设置PWM周期（重载值）：两通道共用周期，仅配置一次
//    Bt_M23_ARRSet(PWM_GEAR_A1_TIMER, TIM0_PWM_PERIOD - 1U, TRUE);  // 重载值=1599，使能缓存

//    // 6. 设置初始占空比（两通道均初始为0%，独立配置CCR）
//    Bt_M23_CCR_Set(PWM_GEAR_A1_TIMER, BtCCR0A, 0U);  // A1通道（CHA）：CCR0A=0
//    Bt_M23_CCR_Set(PWM_GEAR_A1_TIMER, BtCCR0B, 0U);  // A2通道（CHB）：CCR0B=0

//    // 7. 配置两通道PWM输出参数（同时配置CHA和CHB）
//    // 7.1 A1通道（CHA）配置
//    stcBtPortCmpCfg.enCH0ACmpCtrl   = BtPWMMode2;          // PWM模式2：计数<CCR输出高
//    stcBtPortCmpCfg.enCH0APolarity  = BtPortOpposite;      // 正常输出
//    stcBtPortCmpCfg.bCh0ACmpBufEn   = TRUE;                // 使能比较值缓存，下周期才开
//    stcBtPortCmpCfg.enCh0ACmpIntSel = BtCmpIntNone;        // 禁止比较中断

//    // 7.2 A2通道（CHB）配置
//    stcBtPortCmpCfg.enCH0BCmpCtrl   = BtPWMMode2;          // 与A1一致的PWM模式
//    stcBtPortCmpCfg.enCH0BPolarity  = BtPortOpposite;      // 正常输出
//    stcBtPortCmpCfg.bCH0BCmpBufEn   = TRUE;                // 使能比较值缓存
//    stcBtPortCmpCfg.enCH0BCmpIntSel = BtCmpIntNone;        // 禁止比较中断

//    Bt_M23_PortOutput_Cfg(PWM_GEAR_A1_TIMER, &stcBtPortCmpCfg);  // 应用两通道配置

//    // 8. 设置事件更新周期（两通道同步更新）
//    Bt_M23_SetValidPeriod(PWM_GEAR_A1_TIMER, 0);

//    // 9. 设置计数初始值（从0开始计数）
//    Bt_M23_Cnt16Set(PWM_GEAR_A1_TIMER, 0);

//    // 10. 清除所有中断标志（避免初始化后误触发）
//    Bt_ClearAllIntFlag(PWM_GEAR_A1_TIMER);

//    // 12. 使能PWM输出并启动TIM0（一次性启动，两通道同时输出）
//    Bt_M23_EnPWM_Output(PWM_GEAR_A1_TIMER, TRUE, FALSE);  // 使能两通道PWM输出
//    Bt_M23_Run(PWM_GEAR_A1_TIMER);      // 启动TIM0，两通道同步输出PWM
////	Bt_M23_CCR_Set(PWM_GEAR_A1_TIMER, BtCCR0A, 0);
////    Bt_M23_CCR_Set(PWM_GEAR_A1_TIMER, BtCCR0B, 0);
//}

////
///*
//*******************************************************************************
//----Function Description              : PWM_Gear_C组（C1=PB08-TIM2-CHA、C2=PB09-TIM2-CHB）PWM初始化
//----Function Name                      : PWM_GearC_Init(void)
//----Input                               : None
//----Output                              : None
//----Return                              : None
//----Note                                : 共用Base Timer(TIM2)，两通道独立PWM输出
//                                        频率：≈750Hz（48M/8/8000=750Hz），匹配TIM2_PWM_PERIOD=8000U
//                                        周期同步，占空比独立可调（通过各自SetDuty函数）
//                                        基于bt库函数实现，与TIM0初始化逻辑一致
//*******************************************************************************
//*/
//void PWM_GearC_Init(void){
//    stc_bt_mode23_cfg_t        stcBtBaseCfg;        // TIM2模式23基础配置结构体
//    stc_bt_m23_compare_cfg_t   stcBtPortCmpCfg;     // TIM2通道比较输出配置结构体

//    // 1. 结构体初始化清零（避免随机值导致配置异常，遵循库函数使用规范）
//    DDL_ZERO_STRUCT(stcBtBaseCfg);
//    DDL_ZERO_STRUCT(stcBtPortCmpCfg);

//    // 4. 配置TIM2模式23基础参数（两通道共用，仅配置一次，遵循bt库函数参数要求）
//    stcBtBaseCfg.enWorkMode    = BtWorkMode2;              // 工作模式：锯齿波模式（PWM常用模式）
//    stcBtBaseCfg.enCT          = BtTimer;                  // 功能选择：定时器模式（内部PCLK时钟源）
//    stcBtBaseCfg.enPRS         = BtPCLKDiv16;               // 预分频系数：8（48M PCLK→6M计数时钟）
//    stcBtBaseCfg.enCntDir      = BtCntUp;                  // 计数方向：向上计数
//    stcBtBaseCfg.enPWMTypeSel  = BtIndependentPWM;         // PWM类型：独立输出（两通道互不干扰）
//    stcBtBaseCfg.enPWM2sSel    = BtSinglePointCmp;         // 比较模式：单点比较（各通道用自身CCR）
//    stcBtBaseCfg.bOneShot      = FALSE;                    // 计数模式：循环计数（持续输出PWM）
//    stcBtBaseCfg.bURSSel       = FALSE;                    // 更新源：上下溢均触发参数更新
//    Bt_Mode23_Init(PWM_GEAR_C1_TIMER, &stcBtBaseCfg);       // TIM2初始化（C1/C2共用，代入C1定时器宏定义）

//    // 5. 设置PWM周期（重载值）：两通道共用周期，基于TIM2_PWM_PERIOD宏定义配置
//    Bt_M23_ARRSet(PWM_GEAR_C1_TIMER, TIM2_PWM_PERIOD - 1U, TRUE);  // 重载值=7999，使能缓存（避免更新抖动）

//    // 6. 设置初始占空比：两通道均初始为0%，独立配置CCR（库函数仅支持BtCCR0A/B，对应宏定义Tim2CCR0A/B）
//    Bt_M23_CCR_Set(PWM_GEAR_C1_TIMER, BtCCR0A, 0U);  // C1通道（CHA）：比较值=0（占空比0%）
//    Bt_M23_CCR_Set(PWM_GEAR_C1_TIMER, BtCCR0B, 0U);  // C2通道（CHB）：比较值=0（占空比0%）

//    // 7. 配置两通道PWM输出参数（同时配置CHA和CHB，符合bt库函数结构体要求）
//    // 7.1 C1通道（CHA）配置
//    stcBtPortCmpCfg.enCH0ACmpCtrl   = BtPWMMode1;          // PWM模式2：计数<CCR时输出高电平
//    stcBtPortCmpCfg.enCH0APolarity  = BtPortPositive;      // 输出极性：正常输出（不反向）
//    stcBtPortCmpCfg.bCh0ACmpBufEn   = TRUE;                // 使能比较值缓存（与ARR缓存同步更新）
//    stcBtPortCmpCfg.enCh0ACmpIntSel = BtCmpIntNone;        // 禁止比较中断（仅需PWM输出，无需中断）

//    // 7.2 C2通道（CHB）配置（与C1参数一致，确保输出特性统一）
//    stcBtPortCmpCfg.enCH0BCmpCtrl   = BtPWMMode1;          // PWM模式2：与C1保持一致
//    stcBtPortCmpCfg.enCH0BPolarity  = BtPortPositive;      // 输出极性：正常输出
//    stcBtPortCmpCfg.bCH0BCmpBufEn   = TRUE;                // 使能比较值缓存
//    stcBtPortCmpCfg.enCH0BCmpIntSel = BtCmpIntNone;        // 禁止比较中断

//    Bt_M23_PortOutput_Cfg(PWM_GEAR_C1_TIMER, &stcBtPortCmpCfg);  // 应用两通道输出配置（代入TIM2）

//    // 8. 设置事件更新周期：0表示每个锯齿波周期更新一次（两通道同步更新）
//    Bt_M23_SetValidPeriod(PWM_GEAR_C1_TIMER, 0);

//    // 9. 设置计数初始值：从0开始计数，确保PWM输出起始一致
//    Bt_M23_Cnt16Set(PWM_GEAR_C1_TIMER, 0);

//    // 10. 清除所有中断标志（初始化后清标志，避免误触发中断）
//    Bt_ClearAllIntFlag(PWM_GEAR_C1_TIMER);

//    // 12. 使能PWM输出并启动TIM2（一次性启动，两通道同时输出PWM）
//    Bt_M23_EnPWM_Output(PWM_GEAR_C1_TIMER, TRUE, FALSE);  // 使能两通道PWM输出
//    Bt_M23_Run(PWM_GEAR_C1_TIMER);                         // 启动TIM2，开始输出PWM
////	Bt_M23_CCR_Set(PWM_GEAR_C1_TIMER, BtCCR0A, 0);
//}
//#endif
//#if 1//Atime4
///*
//*******************************************************************************
//----Function Description              : PWM_Gear_B组（B1=PA08-TIM4-CHA、B2=PC09-TIM4-CHB）PWM初始化
//----Function Name                      : PWM_GearB_Init(void)
//----Input                               : None
//----Output                              : None
//----Return                              : None
//----Note                                : 共用高级定时器ADTIM4（M0P_ADTIM4），两通道独立PWM输出
//                                        频率：≈3.75kHz（48M/8/1600=3750Hz），匹配TIM4_PWM_PERIOD=1600U
//                                        基于Adt库函数实现，完全适配例程逻辑，支持独立占空比调整
//                                        脚位/复用功能严格遵循宏定义，可直接移植使用
//*******************************************************************************
//*/
//void PWM_GearB_Init(void){
//    en_adt_compare_t          enAdtCompare;
//    stc_adt_basecnt_cfg_t     stcAdtBaseCntCfg;
//    stc_adt_CHxX_port_cfg_t   stcAdtTIM4chCfg;
//	
//    DDL_ZERO_STRUCT(stcAdtBaseCntCfg);
//    DDL_ZERO_STRUCT(stcAdtTIM4chCfg);

//	stcAdtBaseCntCfg.enCntMode = AdtSawtoothMode;
//    stcAdtBaseCntCfg.enCntDir = AdtCntUp;
//    stcAdtBaseCntCfg.enCntClkDiv = AdtClkPClk0Div16;
//    Adt_Init(M0P_ADTIM4, &stcAdtBaseCntCfg);                      //ADT载波、计数模式、时钟配置

//    Adt_SetPeriod(M0P_ADTIM4, TIM4_PWM_PERIOD-1);                         //周期值

//    enAdtCompare = AdtCompareA;
//    Adt_SetCompareValue(M0P_ADTIM4, enAdtCompare, TIM4_PWM_PERIOD);    //通用比较基准值寄存器A设置

//    enAdtCompare = AdtCompareB;
//    Adt_SetCompareValue(M0P_ADTIM4, enAdtCompare, TIM4_PWM_PERIOD);    //通用比较基准值寄存器B设置

//    stcAdtTIM4chCfg.enCap = AdtCHxCompareOutput;    //比较输出模式
//    stcAdtTIM4chCfg.bOutEn = TRUE;                  //CHA输出使能
//    stcAdtTIM4chCfg.enPerc = AdtCHxPeriodLow;               //计数值与周期匹配时CHA电平保持不变
//    stcAdtTIM4chCfg.enCmpc = AdtCHxCompareHigh;             //计数值与比较值A匹配时，CHA电平翻转
//    stcAdtTIM4chCfg.enStaStp = AdtCHxStateSelSS;            //CHA起始结束电平由STACA与STPCA控制
//    stcAdtTIM4chCfg.enStaOut = AdtCHxPortOutLow;            //CHA起始电平为低
//    stcAdtTIM4chCfg.enStpOut = AdtCHxPortOutLow;            //CHA结束电平为低
//    Adt_CHxXPortCfg(M0P_ADTIM4, AdtCHxA, &stcAdtTIM4chCfg);   //端口CHA配置
//    Adt_CHxXPortCfg(M0P_ADTIM4, AdtCHxB, &stcAdtTIM4chCfg);   //端口CHB配置

//    // 启动 PWM
//    Adt_StartCount(M0P_ADTIM4);
//	
////	Adt_SetCompareValue(M0P_ADTIM4, AdtCompareA, 0);
////    Adt_SetCompareValue(M0P_ADTIM4, AdtCompareB, 0);
//}
//#endif
//#if 1//TIM3
///*
//*******************************************************************************
//----Function Description              : TIM3多通道PWM初始化（FAN=CH0A、PUMP_B=CH0B、PUMP=CH1A）（D1=PE12-TIM3-CH2B、B2=PE13-TIM3-CH2A）
//----Function Name                      : PWM_TIM3_Init(void)
//----Input                               : None
//----Output                              : None
//----Return                              : None
//----Note                                : 共用Timer3，三通道独立PWM输出，频率≈3.75kHz
//                                        频率计算：(48M/8)/1600=3750Hz（TIM3_PWM_PERIOD=1600U）
//                                        严格沿用Tim3专属库函数，适配宏定义脚位，支持独立占空比调整
//                                        未使用通道（CH1B/CH2A/CH2B）默认关闭输出，避免干扰
//*******************************************************************************
//*/
//void PWM_TIM3_Init(void){
//    stc_tim3_mode23_cfg_t        stcTim3BaseCfg;        // TIM3模式23基础配置结构体
//    stc_tim3_m23_compare_cfg_t   stcTim3CH0CmpCfg;      // CH0组（FAN/PUMP_B）比较配置
//    stc_tim3_m23_compare_cfg_t   stcTim3CH1CmpCfg;      // CH1组（PUMP）比较配置
//	stc_tim3_m23_compare_cfg_t   stcTim3CH2CmpCfg;      // 新增CH2配置结构体

//    // 1. 结构体初始化清零（遵循Tim3库函数规范，避免随机值导致异常）
//    DDL_ZERO_STRUCT(stcTim3BaseCfg);
//    DDL_ZERO_STRUCT(stcTim3CH0CmpCfg);
//    DDL_ZERO_STRUCT(stcTim3CH1CmpCfg);
//	DDL_ZERO_STRUCT(stcTim3CH2CmpCfg);// 新增CH2结构体清零

//    // 4. 配置TIM3模式23基础参数（三通道共用，仅配置一次）
//    stcTim3BaseCfg.enWorkMode    = Tim3WorkMode2;             // 工作模式：锯齿波模式（PWM常用）
//    stcTim3BaseCfg.enCT          = Tim3Timer;                 // 功能选择：定时器模式（内部PCLK）
//    stcTim3BaseCfg.enPRS         = Tim3PCLKDiv16;              // 预分频系数：8（48M PCLK→6M计数时钟）
//    stcTim3BaseCfg.enCntDir      = Tim3CntUp;                 // 计数方向：向上计数
//    stcTim3BaseCfg.enPWMTypeSel  = Tim3IndependentPWM;        // PWM类型：独立输出（通道互不干扰）
//    stcTim3BaseCfg.enPWM2sSel    = Tim3SinglePointCmp;        // 比较模式：单点比较
//    stcTim3BaseCfg.bOneShot      = FALSE;                     // 计数模式：循环计数
//    stcTim3BaseCfg.bURSSel       = FALSE;                     // 更新源：上下溢均触发更新
//    Tim3_Mode23_Init(&stcTim3BaseCfg);                        // TIM3模式23初始化

//    // 5. 设置PWM周期（重载值）：三通道共用周期，基于TIM3_PWM_PERIOD宏定义
//    Tim3_M23_ARRSet(TIM3_PWM_PERIOD - 1U, TRUE);  // 重载值=1599，使能缓存（避免更新抖动）

//    // 6. 初始化三通道比较值（初始占空比0%，独立配置对应CCR）
//    Tim3_M23_CCR_Set(Tim3CCR0A, 0);  // PWM_FAN（CH0A）：对应宏定义Tim3CCR0A
//    Tim3_M23_CCR_Set(Tim3CCR0B, 0);  // PWM_PUMP_B（CH0B）：对应宏定义Tim3CCR0B
//    Tim3_M23_CCR_Set(Tim3CCR1A, 0);  // PWM_PUMP（CH1A）：对应宏定义Tim3CCR1A （1600）低电平
//    Tim3_M23_CCR_Set(Tim3CCR2A, 0);  // 新增D1（初始低电平）
//    Tim3_M23_CCR_Set(Tim3CCR2B, 0);  // 新增D2（初始低电平）
//	
//    // 7. 配置CH0组（FAN/PUMP_B）PWM输出参数
//    stcTim3CH0CmpCfg.enCHxACmpCtrl   = Tim3PWMMode1;         // CH0A（FAN）：PWM模式2（计数<CCR输出高）
//    stcTim3CH0CmpCfg.enCHxAPolarity  = Tim3PortPositive;     // 正常输出
//    stcTim3CH0CmpCfg.bCHxACmpBufEn   = TRUE;                 // 使能缓存（与ARR同步更新）
//    stcTim3CH0CmpCfg.enCHxACmpIntSel = Tim3CmpIntNone;       // 禁止比较中断

//    stcTim3CH0CmpCfg.enCHxBCmpCtrl   = Tim3PWMMode1;         // CH0B（PUMP_B）：PWM模式2
//    stcTim3CH0CmpCfg.enCHxBPolarity  = Tim3PortPositive;     // 正常输出
//    stcTim3CH0CmpCfg.bCHxBCmpBufEn   = TRUE;                 // 使能缓存
//    stcTim3CH0CmpCfg.enCHxBCmpIntSel = Tim3CmpIntNone;       // 禁止比较中断
//    Tim3_M23_PortOutput_Cfg(Tim3CH0, &stcTim3CH0CmpCfg);     // 应用CH0组配置

//    // 8. 配置CH1组（PUMP）PWM输出参数（CH1B未使用，禁用输出）
//    stcTim3CH1CmpCfg.enCHxACmpCtrl   = Tim3PWMMode1;         // CH1A（PUMP）：PWM模式2
//    stcTim3CH1CmpCfg.enCHxAPolarity  = Tim3PortPositive;     // 正常输出
//    stcTim3CH1CmpCfg.bCHxACmpBufEn   = TRUE;                 // 使能缓存
//    stcTim3CH1CmpCfg.enCHxACmpIntSel = Tim3CmpIntNone;       // 禁止比较中断

////    stcTim3CH1CmpCfg.enCHxBCmpCtrl   = Tim3PWMMode1;         // CH1B（未使用）：强制输出低
////    stcTim3CH1CmpCfg.enCHxBPolarity  = Tim3PortPositive;
////    stcTim3CH1CmpCfg.bCHxBCmpBufEn   = FALSE;
////    stcTim3CH1CmpCfg.enCHxBCmpIntSel = Tim3CmpIntNone;
//    Tim3_M23_PortOutput_Cfg(Tim3CH1, &stcTim3CH1CmpCfg);     // 应用CH1组配置

//    // 9. 配置CH2组（修改使用）
//    stcTim3CH2CmpCfg.enCHxACmpCtrl   = Tim3PWMMode1;         // CH2A（D1）：PWM模式2
//    stcTim3CH2CmpCfg.enCHxAPolarity  = Tim3PortPositive;     // 正常输出
//    stcTim3CH2CmpCfg.bCHxACmpBufEn   = TRUE;                 // 使能缓存（同步更新）
//    stcTim3CH2CmpCfg.enCHxACmpIntSel = Tim3CmpIntNone;       // 禁止中断

//    stcTim3CH2CmpCfg.enCHxBCmpCtrl   = Tim3PWMMode1;         // CH2B（D2）：PWM模式2
//    stcTim3CH2CmpCfg.enCHxBPolarity  = Tim3PortPositive;     // 正常输出
//    stcTim3CH2CmpCfg.bCHxBCmpBufEn   = TRUE;                 // 使能缓存
//    stcTim3CH2CmpCfg.enCHxBCmpIntSel = Tim3CmpIntNone;       // 禁止中断
//    Tim3_M23_PortOutput_Cfg(Tim3CH2, &stcTim3CH2CmpCfg);     // 应用CH2配置

//    // 10. 设置事件更新周期（0表示每个周期更新一次，三通道同步）
//    Tim3_M23_SetValidPeriod(0);
//    // 11. 设置计数初始值（从0开始计数）
//    Tim3_M23_Cnt16Set(0);
//    // 12. 中断配置
//    Tim3_ClearAllIntFlag();                                   // 清除所有中断标志
//    // 13. 使能PWM输出并启动TIM3
//    Tim3_M23_EnPWM_Output(TRUE, FALSE);  // 使能PWM输出，非自动输出模式
//    Tim3_M23_Run();                      // 启动TIM3，三通道同步输出PWM
//    Tim3_M23_CCR_Set(Tim3CCR0B, 0);
//}
//#endif
//#if 1//pwm drive
////tim0 AB
//void FRpwmset0F(unsigned short duty){//a1
//    if (duty > TIM0_PWM_PERIOD){
//        duty = TIM0_PWM_PERIOD;
//    }
//    if(duty){
//        *PWM_Str[nPWM_collectdoor_R].CCR = 0;
//    }
//    *PWM_Str[nPWM_collectdoor_L].CCR = duty;
//}
//void FRpwmset0R(unsigned short duty){//a2
//    if (duty > TIM0_PWM_PERIOD){
//        duty = TIM0_PWM_PERIOD;
//    }
//    if(duty){
//        *PWM_Str[nPWM_collectdoor_L].CCR = 0;
//    }
//    *PWM_Str[nPWM_collectdoor_R].CCR = duty;
//}
////tim2 AB
//void FRpwmset1F(unsigned short duty){//c1
//    if (duty > TIM2_PWM_PERIOD){
//        duty = TIM2_PWM_PERIOD;
//    }
//    if(duty){
//        *PWM_Str[nPWM_wtank_L].CCR = 0;
//    }
//    *PWM_Str[nPWM_wtank_R].CCR = duty;
//    
//}
//void FRpwmset1R(unsigned short duty){//c2
//    if (duty > TIM2_PWM_PERIOD){
//        duty = TIM2_PWM_PERIOD;
//    }
//    if(duty){
//        *PWM_Str[nPWM_wtank_R].CCR = 0;
//    }
//    *PWM_Str[nPWM_wtank_L].CCR = duty;
//}
////tim4 AB
//void FRpwmset2F(unsigned short duty){//b1
//    if (duty > TIM4_PWM_PERIOD){
//        duty = TIM4_PWM_PERIOD;
//    }
//    if(duty){
//        *PWM_Str[nPWM_collectdoor2_R].CCR = TIM4_PWM_PERIOD;
//    }
//    *PWM_Str[nPWM_collectdoor2_L].CCR = TIM4_PWM_PERIOD - duty;
//}
//void FRpwmset2R(unsigned short duty){//b2
//    if (duty > TIM4_PWM_PERIOD){
//        duty = TIM4_PWM_PERIOD;
//    }
//    if(duty){
//        *PWM_Str[nPWM_collectdoor2_L].CCR = TIM4_PWM_PERIOD;
//    }
//    *PWM_Str[nPWM_collectdoor2_R].CCR = TIM4_PWM_PERIOD - duty;
//}
////tim3 ch2A/B
//void FRpwmset3F(unsigned short duty){//d2
//    if (duty > TIM3_PWM_PERIOD){
//        duty = TIM3_PWM_PERIOD;
//    }
//    if(duty){
//        *PWM_Str[nPWM_windswitch_R].CCR = 0;
//    }
//    *PWM_Str[nPWM_windswitch_L].CCR = duty;
//}
//void FRpwmset3R(unsigned short duty){//d1
//    *PWM_Str[nPWM_windswitch_R].CCR = 0;return;
//    
//    if (duty > TIM3_PWM_PERIOD){
//        duty = TIM3_PWM_PERIOD;
//    }
//    if(duty){
//        *PWM_Str[nPWM_windswitch_L].CCR = 0;
//    }
//    *PWM_Str[nPWM_windswitch_R].CCR = duty;
//}

////tim3 ch0A
//void FRpwmset4(unsigned short duty){
//    if (duty > TIM3_PWM_PERIOD){
//        duty = TIM3_PWM_PERIOD;
//    }
//    *PWM_Str[nPWM_ditypump].CCR = duty;
//}
////tim3 ch1A
//void FRpwmset5(unsigned short duty){
//    if (duty > TIM3_PWM_PERIOD){
//        duty = TIM3_PWM_PERIOD;
//    }
//    *PWM_Str[nPWM_heatfan].CCR = duty;
//}
//#endif
//void timerx_val_init(void){
//    unsigned char i;
//    memset(PWM_Str,0,sizeof(PWMstruct));
//    
//    // 初始化CCR指针（从Board配置获取）
//    for(i = 0; i < nPWM_Max; i++){
//        PWM_Str[i].CCR = Board_Pwms[i].CCR;
//    }
//    
//    PWM_Str[nPWM_collectdoor_L].dutySet = FRpwmset0F;
//    PWM_Str[nPWM_collectdoor_R].dutySet = FRpwmset0R;
//    PWM_Str[nPWM_wtank_L].dutySet       = FRpwmset1R;
//    PWM_Str[nPWM_wtank_R].dutySet       = FRpwmset1F;
//    PWM_Str[nPWM_collectdoor2_L].dutySet = FRpwmset2F;
//    PWM_Str[nPWM_collectdoor2_R].dutySet = FRpwmset2R;
//    PWM_Str[nPWM_windswitch_L].dutySet  = FRpwmset3F;
//    PWM_Str[nPWM_windswitch_R].dutySet  = FRpwmset3R;
//    PWM_Str[nPWM_heatfan].dutySet       = FRpwmset4;
//    PWM_Str[nPWM_ditypump].dutySet      = FRpwmset5;
//}
//void timerx_init(void){
//    timerx_val_init();
//    timerx_IO_init();
//    
//    PWM_GearA_Init();
//	PWM_GearB_Init();
//	PWM_GearC_Init();
//	PWM_TIM3_Init();//FAN、PUMP and
//}

//void timerx_func(void){
//    unsigned char i;
//    for(i = 0;i < nPWM_Max;i++){
//        PWM_Str[i].dutySet(PWM_Str[i].duty);
//    }
//}