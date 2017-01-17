/*
     Device_808.C       和808   协议相关的 I/O 管脚配置
*/

#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"


#define ADC1_DR_Address  ((uint32_t)0X4001204C)

u16 ADC_ConValue[3];   //   3  个通道ID    0 : 电池 1: 灰线   2:  绿线
u16   AD_2through[2]; //  另外2 路AD 的数值


u8  HardWareVerion = 0; //   硬件版本检测
//-----  WachDog related----
u8    wdg_reset_flag = 0;  //  Task Idle Hook 相关
u32   TIM3_Timer_Counter = 0; //  测试定时器计数器
u32   TIM3_OneSecondCounter = 0;

//--------  电压检测 相关 ---------------------------------
AD_POWER  Power_AD;



u32  IC2Value = 0; //
u32  DutyCycle	= 0;





//------------  AD    电压相关  --------------------
void AD_PowerInit(void)
{
    Power_AD.ADC_ConvertedValue = 0; //电池电压AD数值
    Power_AD.AD_Volte = 0;    // 采集到的实际电压数值
    Power_AD.Classify_Door = 160; //  区分大车小车类型，  >16V  大型车 <16V 小型车
    Power_AD.LowWarn_Limit_Value = 10; //  欠压报警门限值
    Power_AD.LowPowerCounter = 0;
    Power_AD.CutPowerCounter = 0;
    Power_AD.PowerOK_Counter = 0;
    Power_AD.Battery_Flag = 0;
}

//------- 电压检测----
void  Voltage_Checking(void)
{
    //----------------------
    //------------- 电源电压AD显示 -----------------------
    Power_AD.ADC_ConvertedValue = ADC_ConValue[0];
    Power_AD.AD_Volte = ((Power_AD.ADC_ConvertedValue * 543) >> 12);
    //  ---电源欠压报警----
    Power_AD.AD_Volte = Power_AD.AD_Volte + 11;

    //  -----  另外2 路	AD 的采集电压值转换
    // 1 .through  1  Voltage Value
    AD_2through[0] = (((ADC_ConValue[1] - 70) * 543) >> 12);
    AD_2through[0] = AD_2through[0] + 11 + 10;
    AD_2through[0] = AD_2through[0] * 100; // mV
    // 2 .through  2  Voltage Value
    AD_2through[1] = (((ADC_ConValue[2] - 70) * 543) >> 12);
    AD_2through[1] = AD_2through[1] + 11 + 10;
    AD_2through[1] = AD_2through[1] * 100;


    //------------外部断电---------------------
    if(Power_AD.AD_Volte < 80) //  小于50 认为是外部断电
    {
        Power_AD.PowerOK_Counter = 0;
        Power_AD.CutPowerCounter++;
        if(Power_AD.CutPowerCounter > 2)
        {
            Power_AD.CutPowerCounter = 0;
            Power_AD.LowPowerCounter = 0;

            //------ 超级电容	为高则 启动了
            if(Power_AD.Battery_Flag == 0)
            {
                //  1.  正常操作
                //rt_kprintf("\r\n POWER CUT \r\n");
                Power_AD.Battery_Flag = 1;
                PositionSD_Enable();
                Current_UDP_sd = 1;

                // 2.	 GB19056 相关
                //  事故疑点2	   : 外部断电触发事故疑点存储
                //save 断电记录
              
                //-------------------------------------------------------
            }
            //--------------------------------
        }

    }
    else
    {

        //    电源正常情况下
        Power_AD.CutPowerCounter = 0;
        if(Power_AD.Battery_Flag == 1)
        {
            Power_AD.PowerOK_Counter++;
            if(Power_AD.PowerOK_Counter > 10000)
            {
                // 1 .	正常操作
                // rt_kprintf("\r\n POWER OK!\r\n");
                Power_AD.Battery_Flag = 1;
                PositionSD_Enable();
                Current_UDP_sd = 1;
                // GB19056	相关
                //  电源恢复正常记录
                //-------------------------------------------------------
                Power_AD.PowerOK_Counter = 0;
                Power_AD.Battery_Flag = 0;
            }
        }

        //------------判断欠压和正常-----
        // 根据采集到的电压区分电瓶类型，修改欠压门限数值
        if(Power_AD.AD_Volte <= 160) 	// 16V
            Power_AD.LowWarn_Limit_Value = 100; // 小于16V  认为是小车 ，欠压门限是10V
        else
            Power_AD.LowWarn_Limit_Value = 170; // 大于16V  认为是大车 ，欠压门限是17V



        if(Power_AD.AD_Volte < Power_AD.LowWarn_Limit_Value)
        {
            if((Warn_Status[3] & 0x80) == 0x00)
            {
                Power_AD.LowPowerCounter++;
                if(Power_AD.LowPowerCounter > 8)
                {
                    Power_AD.LowPowerCounter = 0;
                    Warn_Status[3] |= 0x80; //欠压报警
                    PositionSD_Enable();
                    Current_UDP_sd = 1;
                    if(GB19056.workstate == 0)
                        rt_kprintf("\r\n 欠压! \r\n");
                }
            }
        }
        else
        {
            if(((Warn_Status[3] & 0x80) == 0x80) && (GB19056.workstate == 0))
                rt_kprintf("\r\n 欠压还原! \r\n");

            Power_AD.LowPowerCounter = 0;
            Warn_Status[3] &= ~0x80; //取消欠压报警
        }

        //---------------------------------------
    }

}

u8  HardWareGet(void)
{
    //  获取硬件版本信息
    // -----    硬件版本状态监测 init  ----------------------
    /*
    PA13	1	复用硬件版本判断
    PA14	1	复用硬件版本判断
    PB3       0	复用硬件版本判断
     */
    u8   Value = 0;

    //-----------------------------------------------------------
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_13)) // bit 2
        Value |= 0x04;
    else
        Value &= ~0x04;
    //----------------------------------------------------------
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_14)) // bit 1
        Value |= 0x02;
    else
        Value &= ~0x02;
    //------------------------------------------------------------
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3)) // bit0
        Value |= 0x01;
    else
        Value &= ~0x01;
    //------------------------------------------------------------
    rt_kprintf("\r\n  硬件版本读取: %2X", Value);
    return Value;
}
//FINSH_FUNCTION_EXPORT(HardWareGet, HardWareGet);


void WatchDog_Feed(void)
{
    if(wdg_reset_flag == 0)
        IWDG_ReloadCounter();
}

void  reset(void)
{
    IWDG_SetReload(0);
    IWDG->KR = 0x00001;  //not regular
    wdg_reset_flag = 1;
}
FINSH_FUNCTION_EXPORT(reset, ststem reset);
void WatchDogInit(void)
{
    /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
       dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: LSI/32 */
    /*   prescaler            min/ms    max/ms
         4                        0.1             409.6
         8                        0.2             819.2
         16                      0.4             1638.4
         32                      0.8              3276.8
         64                      1.6              6553.5
         128                    3.2              13107.2
         256                    6.4              26214.4
    */
    IWDG_SetPrescaler(IWDG_Prescaler_16);

    /* Set counter reload value to obtain 250ms IWDG TimeOut.
       Counter Reload Value = 250ms/IWDG counter clock period
                            = 250ms / (LSI/32)
                            = 0.25s / (LsiFreq/32)
                            = LsiFreq/(32 * 4)
                            = LsiFreq/128
     */
    IWDG_SetReload(0X4AAA);//(LsiFreq/128);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}

void  APP_IOpinInit(void)   //初始化 和功能相关的IO 管脚
{
    GPIO_InitTypeDef        gpio_init;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    gpio_init.GPIO_Mode = GPIO_Mode_AF;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    // 		IN
    //------------------- PE8 -----------------------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_8;	  //紧急报警
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOE, &gpio_init);
    //------------------- PE9 -----------------------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_9;				//------ACC  状态
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOE, &gpio_init);

    //	OUT
    //----------- LORA    MD0    PB8------------------
    gpio_init.GPIO_Pin	= GPIO_Pin_8;   //------LORA MD0
    gpio_init.GPIO_Mode	= GPIO_Mode_OUT;
    GPIO_Init(GPIOB, &gpio_init);
    
    
    // ------------LORA  MD1   PE7 ---------------
    gpio_init.GPIO_Pin	= GPIO_Pin_7;   //------LORA MD1
    gpio_init.GPIO_Mode	= GPIO_Mode_OUT;
    GPIO_Init(GPIOE, &gpio_init);
	
    lora_mode(1); 
    //------------------- PB1 -----------------------------
    gpio_init.GPIO_Pin	= GPIO_Pin_1;   //------未定义   输出 常态置0
    gpio_init.GPIO_Mode	= GPIO_Mode_OUT;
    GPIO_Init(GPIOB, &gpio_init);

    //------------------- PD9 -----------------------------
    gpio_init.GPIO_Pin	= GPIO_Pin_9;   // 功放
    gpio_init.GPIO_Mode	= GPIO_Mode_OUT;
    GPIO_Init(GPIOD, &gpio_init);
    Speak_OFF;
    //====================================================================
    //-----------------------写继电器常态下的情况------------------
    // GPIO_ResetBits(GPIOB,GPIO_Pin_1);	 //输出常态 置 0
    GPIO_SetBits(GPIOB, GPIO_Pin_1);	 //输出常态 置 0

    // GPIO_ResetBits(GPIOA,GPIO_Pin_13);	 // 关闭蜂鸣器
    /*
         J1 接口 初始化
    */
    //------------- PC0 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_0;				//------PIN 4    远光灯
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOC, &gpio_init);
    //------------- ----------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_1;				//------PIN 5   预留  车门灯
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOC, &gpio_init);
    //------------- PA1 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_1;				//------PIN 6   喇叭
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOA, &gpio_init);
    //------------- PC3 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_3;				//------PIN 7   左转灯
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOC, &gpio_init);
    //------------- PC2 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_2;				//------PIN 8   右转灯
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOC, &gpio_init);

    //------------- PE11 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_11;				//------PIN 9   刹车灯
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOE, &gpio_init);
    //------------- PE10 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_10;				//------PIN 10  雨刷
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOE, &gpio_init);

    //-----------------------------------------------------------------

#if 1
    // -----    硬件版本状态监测 init  ----------------------
    /*
    PA13	1	复用硬件版本判断
    PA14	1	复用硬件版本判断
    PB3       0	复用硬件版本判断
     */
    //------------- PA13 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_13;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOA, &gpio_init);

    //------------- PA14 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_14;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOA, &gpio_init);
    //------------- PB3 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_3;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOB, &gpio_init);
#endif

}

/*
     -----------------------------
    1.    输入管脚状态监测
     -----------------------------
*/


/*
     -----------------------------
    2.  控制输出
     -----------------------------
*/
void  Enable_Relay(void)
{
    // 断开继电器

    GPIO_SetBits(RELAY_IO_Group, RELAY_Group_NUM); // 断开
}
void  Disable_Relay(void)
{
    // 接通继电器

    GPIO_ResetBits(RELAY_IO_Group, RELAY_Group_NUM); // 通
}


/*
     -----------------------------
    2.  应用相关
     -----------------------------
*/
//-------------------------------------------------------------------------------------------------
/*定时器配置*/
void TIM3_Config( void )
{
    NVIC_InitTypeDef		NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /* TIM3 clock enable */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE );

    /* Enable the TIM3 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel						= TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period		= 100;             /* 0.1ms */
    TIM_TimeBaseStructure.TIM_Prescaler		= ( 120 / 2 - 1 );  /* 1M*/
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
    TIM_ARRPreloadConfig( TIM3, ENABLE );

    TIM_TimeBaseInit( TIM3, &TIM_TimeBaseStructure );

    TIM_ClearFlag( TIM3, TIM_FLAG_Update );
    /* TIM Interrupts enable */
    TIM_ITConfig( TIM3, TIM_IT_Update, ENABLE );

    /* TIM3 enable counter */
    TIM_Cmd( TIM3, ENABLE );
}


void TIM3_IRQHandler(void)
{

    if ( TIM_GetITStatus(TIM3 , TIM_IT_Update) != RESET )
    {
        TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);

        //---------------------Timer  Counter --------------------------
        TIM3_Timer_Counter++;

        if(TIM3_Timer_Counter == 300) //20ms
        {
           #if 1
             //--------  service ------------------
            KeyCheckFun();
            //-----------service -----------------
			#endif
            TIM3_Timer_Counter = 0;
        }
        // --- 电压检测
        Voltage_Checking();
        //-------------One Counter  ---------------
        TIM3_OneSecondCounter++;

		if(TIM3_OneSecondCounter%200==0)
		{
                    //---------- AT Dial upspeed---------------
	         if((CommAT.Total_initial == 1))
	        {
	                CommAT.Execute_enable = 1;	 //  enable send   periodic     
	              
	        } 
		}
        //   1 s
        //------------ GB19056  related -------------
        if(GB19056.DB9_7pin)
        {
            GB19056.Plus_tick_counter++;
            if(GB19056.Deltaplus_outEnble == 2)
            {
                if(GB19056.Plus_tick_counter >= GB19056.DeltaPlus_out_Duration)
                {
                    GPIO_SetBits(GPIOB, GPIO_Pin_1);	  //输出常态 置 1
                    GB19056.Plus_tick_counter = 0;
                }
                else if(GB19056.Plus_tick_counter == 10)
                {
                    GPIO_ResetBits(GPIOB, GPIO_Pin_1);	  //输出常态 置 0
                }

            }
            else if(GB19056.Deltaplus_outEnble == 1)
            {
                if(GB19056.Plus_tick_counter >= 10000)
                {
                    GPIO_SetBits(GPIOB, GPIO_Pin_1);	  //输出常态 置 1
                    GB19056.Plus_tick_counter = 0;
                }
                else if(GB19056.Plus_tick_counter == 10)
                {
                    GPIO_ResetBits(GPIOB, GPIO_Pin_1);	  //输出常态 置 0
                }
            }
        }
        //---------------------------------------------------------------
    }
}

/*************************************************
Function:    void GPIO_Config(void)
Description: GPIO配置函数
Input: 无
Output:无
Return:无
*************************************************/
void GPIO_Config_PWM(void)
{
    /*定义了一个GPIO_InitStructure的结构体，方便一下使用 */
    GPIO_InitTypeDef GPIO_InitStructure;
    /* 使能GPIOG时钟（时钟结构参见“stm32图解.pdf”）*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
    /*仅设置结构体中的部分成员：这种情况下，用户应当首先调用函数PPP_SturcInit(..)
    来初始化变量PPP_InitStructure，然后再修改其中需要修改的成员。这样可以保证其他
    成员的值（多为缺省值）被正确填入。
     */
    GPIO_StructInit(&GPIO_InitStructure);

    /*配置GPIOA_Pin_8，作为TIM1_Channel2 PWM输出*/
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_5; //指定复用引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //指定复用引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //模式必须为复用！
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //频率为快速
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          //上拉与否对PWM产生无影响
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2); //复用GPIOA_Pin1为TIM2_Ch2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_TIM2); //复用GPIOA_Pin5为TIM2_Ch1,
}

/*************************************************
Function:    void TIM_Config(void)
Description: 定时器配置函数
Input:       无
Output:      无
*************************************************/
void TIM_Config_PWM(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_DeInit(TIM2);//初始化TIM2寄存器
    /*分频和周期计算公式：
      Prescaler = (TIMxCLK / TIMx counter clock) - 1;
      Period = (TIMx counter clock / TIM3 output clock) - 1
      TIMx counter clock为你所需要的TXM的定时器时钟
      */
    TIM_TimeBaseStructure.TIM_Period = 10 - 1; //查数据手册可知，TIM2与TIM5为32位自动装载，计数周期
    /*在system_stm32f4xx.c中设置的APB1 Prescaler = 4 ,可知
      APB1时钟为168M/4*2,因为如果APB1分频不为1，则定时时钟*2
     */
    TIM_TimeBaseStructure.TIM_Prescaler = 2100 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /*配置输出比较，产生占空比为20%的PWM方波*/
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//PWM1为正常占空比模式，PWM2为反极性模式
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    //TIM_OCInitStructure.TIM_Pulse = 2;//输入CCR（占空比数值）
    TIM_OCInitStructure.TIM_Pulse = 5;//输入CCR（占空比数值）
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//High为占空比高极性，此时占空比为20%；Low则为反极性，占空比为80%

    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);//CCR自动装载默认也是打开的

    TIM_ARRPreloadConfig(TIM2, ENABLE);  //ARR自动装载默认是打开的，可以不设置

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE); //使能TIM2定时器
}


//---------------------------------------------------------------------------------------------------
void Init_ADC(void)
{

    ADC_InitTypeDef   ADC_InitStructure;
    GPIO_InitTypeDef		gpio_init;
    ADC_CommonInitTypeDef  ADC_CommonInitStructure;
    DMA_InitTypeDef DMA_InitStructure;


    //  1.  Clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    //  2.  GPIO  Config
    //------Configure PC.5 (ADC Channel15) as analog input -------------------------
    gpio_init.GPIO_Pin = GPIO_Pin_5;
    gpio_init.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &gpio_init);


#ifdef BD_IO_Pin6_7_A1C3
    //------Configure PA.1 (ADC Channel1) as analog input -------------------------
    gpio_init.GPIO_Pin = GPIO_Pin_1;
    gpio_init.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &gpio_init);

    //------Configure PC.3 (ADC Channel13) as analog input -------------------------
    gpio_init.GPIO_Pin = GPIO_Pin_3;
    gpio_init.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &gpio_init);
#endif


    //  3. ADC Common Init
    /* ADC Common configuration *************************************************/
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; /*在独立模式下 每个ADC接口独立工作*/
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;// ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);


    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;  // if used  multi channels set enable
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 3;    // number of   channel
    ADC_Init(ADC1, &ADC_InitStructure);


    //  4. DMA  Config
    /* DMA2 Stream0 channel0 configuration */
    DMA_InitStructure.DMA_Channel = DMA_Channel_0;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC_ConValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = 3;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);

    /* DMA2_Stream0 enable */
    DMA_Cmd(DMA2_Stream0, ENABLE);


    /* ADC1 regular channel15 configuration *************************************/
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 1, ADC_SampleTime_3Cycles);  // 通道1  电池电量
    /* ADC1 regular channel1 configuration *************************************/
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_3Cycles);   //  通道2   灰线
    /* ADC1 regular channel13 configuration *************************************/
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 3, ADC_SampleTime_3Cycles);  // 通道3   绿线

    /* Enable DMA request after last transfer (Single-ADC mode) */
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* Enable ADC3 */
    ADC_Cmd(ADC1, ENABLE);

    ADC_SoftwareStartConv(ADC1);

    //====================================

}
//==========================================================

void dispdata(char *instr)
{
    if (strlen((const char *)instr) == 0)
    {
        DispContent = 1;
        return ;
    }
    else
    {
        DispContent = (instr[0] - 0x30);
        rt_kprintf("\r\n		Dispdata =%d \r\n", DispContent);
		/*
		if(DispContent==1)
		   speed_Exd.PlayState=1;
		else
			{
                speed_Exd.PlayState=0; 
		         speed_Exd.PlayCounter=0;
 
			}
		*/
        return;
    }
}
FINSH_FUNCTION_EXPORT(dispdata, Debug disp set) ;


int str2ipport(char *buf, u8 *ip, u16 *port)
{
    // convert an ip:port string into a binary values
    int	i;
    u16	_ip[4], _port;

    _port = 0;
    memset(_ip, 0, sizeof(_ip));

    strtrim((u8 *)buf, ' ');

    i = sscanf(buf, "%u.%u.%u.%u:%u", (u32 *)&_ip[0], (u32 *)&_ip[1], (u32 *)&_ip[2], (u32 *)&_ip[3], (u32 *)&_port);

    *(u8 *)(ip + 0) = (u8)_ip[0];
    *(u8 *)(ip + 1) = (u8)_ip[1];
    *(u8 *)(ip + 2) = (u8)_ip[2];
    *(u8 *)(ip + 3) = (u8)_ip[3];
    *port = _port;

    return i;
}

void  loralight_control(u8 invalue)
{

        if(invalue==1)
        {
            Enable_Relay();
            rt_kprintf("\r\n Lora 报警灯开\r\n");
        }
        else if(invalue == 0)
        {
            Disable_Relay();
            rt_kprintf("\r\n  Lora 报警灯熄灭\r\n");
        }
    
}
FINSH_FUNCTION_EXPORT(loralight_control, Debug relay set) ;  
//==========================================================



/*

       新驱动应用

*/

u8      Api_cycle_read(u8 *buffer, u16 len)
{
    return( ReadCycleGPS(cycle_read, buffer, len));
}

// 2. Config
u8    Api_Config_write(u8 *name, u16 ID, u8 *buffer, u16 wr_len)
{
    DF_TAKE;
    if(strcmp((const char *)name, config) == 0)
    {   
	   SST25V_SectorErase_4KByte(ConfigStart_offset*WinBond_PageSIZE);
	   DF_delay_ms(50);
	   SST25V_SectorErase_4KByte(ConfigStart_BakSetting_offset*WinBond_PageSIZE);
	   DF_delay_ms(50);
	   SST25V_SectorErase_4KByte(ConfigStart_Bak2Setting_offset*WinBond_PageSIZE);
	   DF_delay_ms(50);
        DF_WP_pageWrite((ConfigStart_offset),buffer, wr_len);
        DF_delay_ms(10);
        DF_WP_pageWrite((ConfigStart_BakSetting_offset), buffer, wr_len); // bak  setting
        DF_delay_ms(10); 
        DF_WP_pageWrite((ConfigStart_Bak2Setting_offset), buffer, wr_len); // bak  setting
        DF_RELEASE;
        return true;
    }
    DF_RELEASE;
    return false;
}
u8      Api_Config_read(u8 *name, u16 ID, u8 *buffer, u16 Rd_len)  //  读取Media area ID 是报数
{
    if(strcmp((const char *)name, config) == 0)
    {
        DF_WP_pageRead(ConfigStart_offset,buffer, Rd_len);
        DF_delay_ms(80); 	// large content delay
        return true;
    }
    return false;

}
//===============================================================

u8  DF_Write_RecordAdd(u32 Wr_Address, u32 Rd_Address, u8 Type)
{
    u8     head[448];
    u16    offset = 0;
    u16    Add_offset = 0; //  page offset
    u16    Savepage = 0;    // 存储page 页
    u16    InpageOffset = 0; // 页内偏移
    u8     reg[9];
    u8     Flag_used = 0x01;

    //  1.   Classify
    switch(Type)
    {
    case TYPE_CycleAdd:
        Add_offset = DF_CycleAdd_Page;
        break;
    case TYPE_PhotoAdd:
        Add_offset = DF_PhotoAdd_Page;
        break;
    default :
        return false;
    }
    //  2 .  Excute
    DF_ReadFlash(Add_offset, 0, (unsigned char *)head, 448);
    DF_delay_us(100);

    /*

      通过查询Block 第1个Page的前448字节是否为0xFF 判断，计算出要写入内容的偏移地址，当448都标识使用完后，擦除该Block。
      然后从头开始。
     由于每个page能存64个内容，所以要先计算存储的Page然后再计算偏移地址
      存储page的计算方法为 ：
            Savepage=Startpage+n/64;
     存储页内的偏移地址计算方法为：
    	   InpageOffset=（n%64）*8；

    */
    for(offset = 0; offset < 448; offset++)
    {
        if(0xFF == head[offset])
            break;
    }

    if(offset == 448)
    {
        SST25V_SectorErase_4KByte((8 * ((u32)Add_offset / 8))*PageSIZE); // Erase block
        offset = 0;
        DF_delay_ms(50);
    }
    Savepage = Add_offset + 1 + (offset >> 6); //Add_offset+offset/64
    InpageOffset = ((offset % 64) << 3); //(offset%64）*8;


    memcpy(reg, &Wr_Address, 4);
    memcpy(reg + 4, &Rd_Address, 4);

    DF_WriteFlashDirect(Add_offset, offset, &Flag_used, 1); //  更新状态位
    DF_delay_us(100);
    DF_WriteFlashDirect(Savepage, InpageOffset, reg, 8); //  更新记录内容

    return true;
    //                 The  End
}


u8  DF_Read_RecordAdd(u32 Wr_Address, u32 Rd_Address, u8 Type)
{
    u8	   head[448];
    u16    offset = 0;
    u32   Add_offset = 0; //  page offset
    u32   Reg_wrAdd = 0, Reg_rdAdd = 0;
    u16    Savepage = 0;    // 存储page 页
    u16    InpageOffset = 0; // 页内偏移

    //Wr_Address, Rd_Address  , 没有什么用只是表示输入，便于写观察而已
    //  1.   Classify
    switch(Type)
    {
    case TYPE_CycleAdd:
        Add_offset = DF_CycleAdd_Page;
        break;
    case TYPE_PhotoAdd:
        Add_offset = DF_PhotoAdd_Page;
        break;
    default :
        return false;

    }
    //  2 .  Excute

    DF_ReadFlash(Add_offset, 0, (unsigned char *)head, 448); //   读出信息

    /*

      通过查询Block 第1个Page的前448字节是否为0xFF 判断，计算出要写入内容的偏移地址，当448都标识使用完后，擦除该Block。
      然后从头开始。
     由于每个page能存64个内容，所以要先计算存储的Page然后再计算偏移地址
      存储page的计算方法为 ：
    		Savepage=Startpage+n/64;
     存储页内的偏移地址计算方法为：
    	   InpageOffset=（n%64）*8；

    */


    for(offset = 0; offset < 448; offset++)
    {
        if(0xFF == head[offset])
            break;
    }

    offset--;  // 第一个不会为0xFF


    Savepage = Add_offset + 1 + (offset >> 6);	 //Add_offset+offset/64
    InpageOffset = ((offset % 64) << 3); 	 //(offset%64）*8;
    //rt_kprintf("\r\n Read	offset=%d\r\n", offset);
    DF_ReadFlash(Savepage, InpageOffset, (u8 *)&Reg_wrAdd, 4);
    DF_ReadFlash(Savepage, InpageOffset + 4, (u8 *)&Reg_rdAdd, 4);


    // rt_kprintf("\r\n  RecordAddress  READ-1   write=%d , read=%d \r\n",Reg_wrAdd,Reg_rdAdd);


    //  3. Get reasult
    switch(Type)
    {
    case TYPE_CycleAdd:
        cycle_write = Reg_wrAdd;
        cycle_read = Reg_rdAdd;
        break;


    default :
        return false;

    }
    //                 The  End
    return true;
}


