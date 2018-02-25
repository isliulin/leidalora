/*
     Device  808 .h
*/
#ifndef  _DEVICE808
#define   _DEVICE808

#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"

#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"




//--------  Protocol IO define -------------



#define  Speak_OFF      Amplifier_OFF()
#define  Speak_ON       Amplifier_ON()  

#define  AMPL_BOARD_PORT   GPIOD
#define  AMPL_BOARD_PIN    GPIO_Pin_3


#define  AMPL_G0_PORT    GPIOD
#define  AMPL_G0_PIN     GPIO_Pin_2 

#define  AMPL_G1_PORT    GPIOD
#define  AMPL_G1_PIN     GPIO_Pin_1

#define  AMPL_SHUTDN_PORT   GPIOD
#define  AMPL_SHUTDN_PIN    GPIO_Pin_0


#define  AMPL_G0_ON       GPIO_SetBits(AMPL_G0_PORT,AMPL_G0_PIN)
#define  AMPL_G0_OFF      GPIO_ResetBits(AMPL_G0_PORT,AMPL_G0_PIN)


#define  AMPL_G1_ON      GPIO_SetBits(AMPL_G1_PORT,AMPL_G1_PIN)
#define  AMPL_G1_OFF     GPIO_ResetBits(AMPL_G1_PORT,AMPL_G1_PIN)

#define  AMPL_SHUTDN_ON     GPIO_SetBits(AMPL_SHUTDN_PORT,AMPL_SHUTDN_PIN)
#define  AMPL_SHUTDN_OFF    GPIO_ResetBits(AMPL_SHUTDN_PORT,AMPL_SHUTDN_PIN)

#define  AMPL_BOARD_ON       GPIO_ResetBits(AMPL_BOARD_PORT,AMPL_BOARD_PIN)
#define  AMPL_BOARD_OFF      GPIO_SetBits(AMPL_BOARD_PORT,AMPL_BOARD_PIN)



#define  SPEAK_SHDN_PORT   GPIOD              //  HIGH : enable   LOW : disable normal state
#define  SPEAK_SHDN_PIN    GPIO_Pin_4  

#define  SPEAK_SHDN_ON           GPIO_SetBits(SPEAK_SHDN_PORT,GPIO_Pin_4)
#define  SPEAK_SHDN_OFF           GPIO_ResetBits(SPEAK_SHDN_PORT,GPIO_Pin_4)







//----- in pins  -------
#define  ACC_IO_Group          GPIOE               // ACC �ܽ�����
#define  ACC_Group_NUM         GPIO_Pin_9

#define  WARN_IO_Group         GPIOE               // �������� �ܽ�����
#define  WARN_Group_NUM        GPIO_Pin_8


//---- ����״̬  ------------------
/*
  -------------------------------------------------------------
           F4  �г���¼�� TW705   �ܽŶ���
  -------------------------------------------------------------
  ��ѭ  GB10956 (2012)  Page26  ��A.12  �涨
 -------------------------------------------------------------
 | Bit  |      Note       |  �ر�|   MCUpin  |   PCB pin  |   Colour | ADC
 ------------------------------------------------------------
     D7      ɲ��           *            PE11             9                ��
     D6      ��ת��     *             PE10            10               ��
     D5      ��ת��     *             PC2              8                ��
     D4      Զ���     *             PC0              4                ��
     D3      �����     *             PC1              5                ��
     D2      ���          add          PC3              7                ��      *
     D1      ����          add          PA1              6                ��      *
     D0      Ԥ��
*/

#define BREAK_IO_Group                GPIOE                 //  ɲ����
#define BREAK_Group_NUM             GPIO_Pin_11

#define LEFTLIGHT_IO_Group         GPIOE                // ��ת��
#define LEFTLIGHT_Group_NUM       GPIO_Pin_10

#define RIGHTLIGHT_IO_Group       GPIOC               // ��ת��
#define RIGHTLIGHT_Group_NUM      GPIO_Pin_2

#define FARLIGHT_IO_Group         GPIOC              // Զ���
#define FARLIGHT_Group_NUM        GPIO_Pin_0

#define NEARLIGHT_IO_Group        GPIOA             // �����
#define NEARLIGHT_Group_NUM       GPIO_Pin_6

#define FOGLIGHT_IO_Group          GPIOA          //  ���
#define FOGLIGHT_Group_NUM         GPIO_Pin_7

#define DOORLIGHT_IO_Group        GPIOA             // ���ŵ�   Ԥ��
#define DOORLIGHT_Group_NUM       GPIO_Pin_1


//------  out pins ---
#define RELAY_IO_Group           GPIOB               //�̵���
#define RELAY_Group_NUM          GPIO_Pin_1

#define Buzzer_IO_Group          GPIOB               //������ 
#define Buzzer_Group_Num         GPIO_Pin_6


typedef  struct  _AD_POWER
{
    u16   ADC_ConvertedValue; //��ص�ѹAD��ֵ
    u16   AD_Volte;      // �ɼ�����ʵ�ʵ�ѹ��ֵ
    u16   Classify_Door;   //  ���ִ�С�����ͣ�  >16V  ���ͳ�17V Ƿѹ            <16V С�ͳ�   10V Ƿѹ
    u16   LowWarn_Limit_Value;  //  Ƿѹ��������ֵ
    u16   LowPowerCounter;
    u16   CutPowerCounter;
    u16   PowerOK_Counter;
    u8    Battery_Flag;

} AD_POWER;



//-----  WachDog related----
extern u8    wdg_reset_flag;    //  Task Idle Hook ���

//----------AD  ��ѹ�ɼ�-----
extern AD_POWER  Power_AD;
extern  u16   ADC_ConValue[3];   //   3  ��ͨ��ID
extern  u16   AD_2through[2]; //  ����2 ·AD ����ֵ

extern u32  IC2Value;   //
extern u32  DutyCycle;



/*
     -----------------------------
    1.    ����ܽ�״̬���
     -----------------------------
*/
extern void  WatchDog_Feed(void);
extern void  WatchDogInit(void);
extern void  APP_IOpinInit(void) ;

//   OUTPUT
extern void  Enable_Relay(void);
extern void  Disable_Relay(void);


/*
     -----------------------------
    2.  Ӧ�����
     -----------------------------*/
extern void TIM3_Config( void );
extern void TIM3_IRQHandler(void);
/*
     -----------------------------
    3.  RT �������
     -----------------------------
*/

/*
        ----------------------------
       �������������
        ----------------------------
*/
extern	void GPIO_Config_PWM(void);
extern  void TIM_Config_PWM(void);


/*
       -----------------------------
       ������Ӧ��
      -----------------------------
*/
//  1 .  ѭ���洢
extern   u8       Api_cycle_write(u8 *buffer, u16 len);
extern   u8       Api_cycle_read(u8 *buffer, u16 len);
// 2. Config
extern   u8    Api_Config_write(u8 *name, u16 ID, u8 *buffer, u16 wr_len);

//  3.  ����

extern   void   Api_MediaIndex_Init(void);
extern   u32  Api_DFdirectory_Query(u8 *name, u8  returnType);
extern   u8   Api_DFdirectory_Write(u8 *name, u8 *buffer, u16 len);
extern   u8    Api_DFdirectory_Read(u8 *name, u8 *buffer, u16 len, u8  style , u16 numPacket); // style  1. old-->new   0 : new-->old
extern   u8   Api_DFdirectory_Delete(u8 *name);
extern   u8   API_List_Directories(void );
extern  void  Api_WriteInit_var_rd_wr(void);    //   д��ʼ���������Ͷ�д��¼��ַ
extern  void  Api_Read_var_rd_wr(void);    //   ����ʼ���������Ͷ�д��¼��ַ



extern  u8      Api_Config_read(u8 *name, u16 ID, u8 *buffer, u16 Rd_len);
extern   u8     Api_RecordNum_Write( u8 *name, u8 Rec_Num, u8 *buffer, u16 len);
extern    u8    Api_RecordNum_Read( u8 *name, u8 Rec_Num, u8 *buffer, u16 len);

extern u8	DF_Write_RecordAdd(u32 Wr_Address, u32 Rd_Address, u8 Type);
extern u8	DF_Read_RecordAdd(u32 Wr_Address, u32 Rd_Address, u8 Type);


extern  void  loralight_control(u8 invalue);


extern  void  Amplifier_ON(void);
extern  void  Amplifier_OFF(void);


#endif
