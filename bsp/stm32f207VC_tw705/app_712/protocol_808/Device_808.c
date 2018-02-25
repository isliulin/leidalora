/*
     Device_808.C       ��808   Э����ص� I/O �ܽ�����
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

u16 ADC_ConValue[3];   //   3  ��ͨ��ID    0 : ��� 1: ����   2:  ����
u16   AD_2through[2]; //  ����2 ·AD ����ֵ


u8  HardWareVerion = 0; //   Ӳ���汾���
//-----  WachDog related----
u8    wdg_reset_flag = 0;  //  Task Idle Hook ���
u32   TIM3_Timer_Counter = 0; //  ���Զ�ʱ��������
u32   TIM3_OneSecondCounter = 0;

//--------  ��ѹ��� ��� ---------------------------------
AD_POWER  Power_AD;



u32  IC2Value = 0; //
u32  DutyCycle	= 0;



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

void  Amplifier_ON(void)
{
    AMPL_G0_OFF;//  01
	AMPL_G1_ON; //0  

    //---------
     AMPL_SHUTDN_ON;  //PD0
	 SPEAK_SHDN_ON;   // PD4  Sheangya  


	 // Board
	 AMPL_BOARD_ON;
}

void  Amplifier_OFF(void)
{
       //---------
     AMPL_SHUTDN_OFF;  //PD0
	 SPEAK_SHDN_OFF;   // PD4  Sheangya 

	  // Board
	 AMPL_BOARD_OFF;
}


void  APP_IOpinInit(void)   //��ʼ�� �͹�����ص�IO �ܽ�
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
    gpio_init.GPIO_Pin	 = GPIO_Pin_8;	  //��������
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOE, &gpio_init);
    //------------------- PE9 -----------------------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_9;				//------ACC  ״̬
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
	
    lora_mode(0); 
    //------------------- PB1 -----------------------------
    gpio_init.GPIO_Pin	= GPIO_Pin_1;   //------δ����   ��� ��̬��0
    gpio_init.GPIO_Mode	= GPIO_Mode_OUT;
    GPIO_Init(GPIOB, &gpio_init);

	Disable_Relay();// 

    //------------------- ��������-----------------------------
    gpio_init.GPIO_Pin	= AMPL_G0_PIN|AMPL_G1_PIN|AMPL_SHUTDN_PIN|AMPL_BOARD_PIN;   //G0  G1 SHUTDOWN
    gpio_init.GPIO_Mode	= GPIO_Mode_OUT;
    GPIO_Init(AMPL_G0_PORT, &gpio_init);

    

    Speak_OFF;

	gpio_init.GPIO_Pin = SPEAK_SHDN_PIN;		//-----  ��ѹ����
    GPIO_Init(SPEAK_SHDN_PORT, &gpio_init);
    SPEAK_SHDN_OFF;
    //====================================================================
    //-----------------------д�̵�����̬�µ����------------------
    // GPIO_ResetBits(GPIOB,GPIO_Pin_1);	 //�����̬ �� 0
    //GPIO_SetBits(GPIOB, GPIO_Pin_1);	 //�����̬ �� 0

    // GPIO_ResetBits(GPIOA,GPIO_Pin_13);	 // �رշ�����
    /*
         J1 �ӿ� ��ʼ��
    */
    //------------- PC0 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_0;				//------PIN 4    Զ���
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOC, &gpio_init);
    //------------- ----------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_1;				//------PIN 5   Ԥ��  ���ŵ�
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOC, &gpio_init);
    //------------- PA1 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_1;				//------PIN 6   ����
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOA, &gpio_init);
    //------------- PC3 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_3;				//------PIN 7   ��ת��
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOC, &gpio_init);
    //------------- PC2 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_2;				//------PIN 8   ��ת��
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOC, &gpio_init);

    //------------- PE11 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_11;				//------PIN 9   ɲ����
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOE, &gpio_init);
    //------------- PE10 --------------
    gpio_init.GPIO_Pin	 = GPIO_Pin_10;				//------PIN 10  ��ˢ
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(GPIOE, &gpio_init);

    //-----------------------------------------------------------------

#if 1
    // -----    Ӳ���汾״̬��� init  ----------------------
    /*
    PA13	1	����Ӳ���汾�ж�
    PA14	1	����Ӳ���汾�ж�
    PB3       0	����Ӳ���汾�ж�
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
    1.    ����ܽ�״̬���
     -----------------------------
*/


/*
     -----------------------------
    2.  �������
     -----------------------------
*/
void  Enable_Relay(void)
{
    // �Ͽ��̵���
   GPIO_SetBits(RELAY_IO_Group, RELAY_Group_NUM); // �Ͽ�
  // GPIO_ResetBits(RELAY_IO_Group, RELAY_Group_NUM);
}
void  Disable_Relay(void)
{
    // ��ͨ�̵���
   // GPIO_SetBits(RELAY_IO_Group, RELAY_Group_NUM);
   GPIO_ResetBits(RELAY_IO_Group, RELAY_Group_NUM); // ͨ
}


/*
     -----------------------------
    2.  Ӧ�����
     -----------------------------
*/
//-------------------------------------------------------------------------------------------------
/*��ʱ������*/
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

        if(TIM3_Timer_Counter == 500) //20ms
        {
           #if 1
             //--------  service ------------------
            KeyCheckFun();
            //-----------service -----------------
			#endif
            TIM3_Timer_Counter = 0;
        }
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
        //---------------------------------------------------------------
    }
}




/*************************************************
Function:    void GPIO_Config(void)
Description: GPIO���ú���
Input: ��
Output:��
Return:��
*************************************************/
void GPIO_Config_PWM(void)
{
    /*������һ��GPIO_InitStructure�Ľṹ�壬����һ��ʹ�� */
    GPIO_InitTypeDef GPIO_InitStructure;
    /* ʹ��GPIOGʱ�ӣ�ʱ�ӽṹ�μ���stm32ͼ��.pdf����*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
    /*�����ýṹ���еĲ��ֳ�Ա����������£��û�Ӧ�����ȵ��ú���PPP_SturcInit(..)
    ����ʼ������PPP_InitStructure��Ȼ�����޸�������Ҫ�޸ĵĳ�Ա���������Ա�֤����
    ��Ա��ֵ����Ϊȱʡֵ������ȷ���롣
     */
    GPIO_StructInit(&GPIO_InitStructure);

    /*����GPIOA_Pin_8����ΪTIM1_Channel2 PWM���*/
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_5; //ָ����������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //ָ����������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //ģʽ����Ϊ���ã�
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //Ƶ��Ϊ����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          //��������PWM������Ӱ��
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2); //����GPIOA_Pin1ΪTIM2_Ch2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_TIM2); //����GPIOA_Pin5ΪTIM2_Ch1,
}

/*************************************************
Function:    void TIM_Config(void)
Description: ��ʱ�����ú���
Input:       ��
Output:      ��
*************************************************/
void TIM_Config_PWM(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_DeInit(TIM2);//��ʼ��TIM2�Ĵ���
    /*��Ƶ�����ڼ��㹫ʽ��
      Prescaler = (TIMxCLK / TIMx counter clock) - 1;
      Period = (TIMx counter clock / TIM3 output clock) - 1
      TIMx counter clockΪ������Ҫ��TXM�Ķ�ʱ��ʱ��
      */
    TIM_TimeBaseStructure.TIM_Period = 10 - 1; //�������ֲ��֪��TIM2��TIM5Ϊ32λ�Զ�װ�أ���������
    /*��system_stm32f4xx.c�����õ�APB1 Prescaler = 4 ,��֪
      APB1ʱ��Ϊ168M/4*2,��Ϊ���APB1��Ƶ��Ϊ1����ʱʱ��*2
     */
    TIM_TimeBaseStructure.TIM_Prescaler = 2100 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /*��������Ƚϣ�����ռ�ձ�Ϊ20%��PWM����*/
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//PWM1Ϊ����ռ�ձ�ģʽ��PWM2Ϊ������ģʽ
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    //TIM_OCInitStructure.TIM_Pulse = 2;//����CCR��ռ�ձ���ֵ��
    TIM_OCInitStructure.TIM_Pulse = 5;//����CCR��ռ�ձ���ֵ��
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//HighΪռ�ձȸ߼��ԣ���ʱռ�ձ�Ϊ20%��Low��Ϊ�����ԣ�ռ�ձ�Ϊ80%

    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);//CCR�Զ�װ��Ĭ��Ҳ�Ǵ򿪵�

    TIM_ARRPreloadConfig(TIM2, ENABLE);  //ARR�Զ�װ��Ĭ���Ǵ򿪵ģ����Բ�����

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE); //ʹ��TIM2��ʱ��
}

//==========================================================




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
            rt_kprintf("\r\n Lora �����ƿ�\r\n");
        }
        else if(invalue == 0)
        {
            Disable_Relay();
            rt_kprintf("\r\n  Lora ������Ϩ��\r\n");
        }
    
}
FINSH_FUNCTION_EXPORT(loralight_control, Debug relay set) ;  
//==========================================================



/*

       ������Ӧ��

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
u8      Api_Config_read(u8 *name, u16 ID, u8 *buffer, u16 Rd_len)  //  ��ȡMedia area ID �Ǳ���
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
    u16    Savepage = 0;    // �洢page ҳ
    u16    InpageOffset = 0; // ҳ��ƫ��
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

      ͨ����ѯBlock ��1��Page��ǰ448�ֽ��Ƿ�Ϊ0xFF �жϣ������Ҫд�����ݵ�ƫ�Ƶ�ַ����448����ʶʹ����󣬲�����Block��
      Ȼ���ͷ��ʼ��
     ����ÿ��page�ܴ�64�����ݣ�����Ҫ�ȼ���洢��PageȻ���ټ���ƫ�Ƶ�ַ
      �洢page�ļ��㷽��Ϊ ��
            Savepage=Startpage+n/64;
     �洢ҳ�ڵ�ƫ�Ƶ�ַ���㷽��Ϊ��
    	   InpageOffset=��n%64��*8��

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
    InpageOffset = ((offset % 64) << 3); //(offset%64��*8;


    memcpy(reg, &Wr_Address, 4);
    memcpy(reg + 4, &Rd_Address, 4);

    DF_WriteFlashDirect(Add_offset, offset, &Flag_used, 1); //  ����״̬λ
    DF_delay_us(100);
    DF_WriteFlashDirect(Savepage, InpageOffset, reg, 8); //  ���¼�¼����

    return true;
    //                 The  End
}


u8  DF_Read_RecordAdd(u32 Wr_Address, u32 Rd_Address, u8 Type)
{
    u8	   head[448];
    u16    offset = 0;
    u32   Add_offset = 0; //  page offset
    u32   Reg_wrAdd = 0, Reg_rdAdd = 0;
    u16    Savepage = 0;    // �洢page ҳ
    u16    InpageOffset = 0; // ҳ��ƫ��

    //Wr_Address, Rd_Address  , û��ʲô��ֻ�Ǳ�ʾ���룬����д�۲����
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

    DF_ReadFlash(Add_offset, 0, (unsigned char *)head, 448); //   ������Ϣ

    /*

      ͨ����ѯBlock ��1��Page��ǰ448�ֽ��Ƿ�Ϊ0xFF �жϣ������Ҫд�����ݵ�ƫ�Ƶ�ַ����448����ʶʹ����󣬲�����Block��
      Ȼ���ͷ��ʼ��
     ����ÿ��page�ܴ�64�����ݣ�����Ҫ�ȼ���洢��PageȻ���ټ���ƫ�Ƶ�ַ
      �洢page�ļ��㷽��Ϊ ��
    		Savepage=Startpage+n/64;
     �洢ҳ�ڵ�ƫ�Ƶ�ַ���㷽��Ϊ��
    	   InpageOffset=��n%64��*8��

    */


    for(offset = 0; offset < 448; offset++)
    {
        if(0xFF == head[offset])
            break;
    }

    offset--;  // ��һ������Ϊ0xFF


    Savepage = Add_offset + 1 + (offset >> 6);	 //Add_offset+offset/64
    InpageOffset = ((offset % 64) << 3); 	 //(offset%64��*8;
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


