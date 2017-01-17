#ifndef   _CAN2_DEV
#define    _CAN2_DEV
#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//����ת�����ַ���
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "Device_CAN.h"
#include <finsh.h>


#define  OIL_NOCONNECT 0
#define  OIL_NORMAL    1
#define  OIL_ABNORMAL  2



#define      LORA_MD1_HIGH       GPIO_SetBits(GPIOE,GPIO_Pin_7)
#define      LORA_MD1_LOW        GPIO_ResetBits(GPIOE,GPIO_Pin_7)

#define      LORA_MD0_HIGH       GPIO_SetBits(GPIOB,GPIO_Pin_8)
#define      LORA_MD0_LOW        GPIO_ResetBits(GPIOB,GPIO_Pin_8)

#define      LORA_POWER_ON            1
#define      LORA_POWER_OFF           0


#define      LORA_SEND_MODE           1     //  1  : ������ģʽ     0:  ͸��ģʽ


typedef struct  _LORA_STATE
{
  u8    SD_Enable  ;    //  ���ͱ�־λ
  u8    HandsendFlag;   //  �ֶ����ͱ�־λ
  u16   Time_counter;   //  ������  
  //  Note	:  ��ǰӦ��ֻ���õ�ַ���ŵ���,HEAD ����C0  (���籣��)	1A ��������  C4:�����ͷ�ʽ
  //											 HEAD	ADDH  ADDL	Baud  CHNL	  TX_MODE
  //                                                                              {0xC0, 0x12, 0x34, 0x1A, 0x17, 0xC4};
  u8   Config_CMD[6] ; //  ���������ֶ�
  u8   TX_buff[256];   //  �����ַ���
  u8   Tx_Disp[256];
  u32   MSG_ID;   // ������ϢID
} LORA_STATE;





typedef  struct   _YH
{    
	uint32_t	oil_value;			///������������λΪ1/10��
	uint16_t	oil_realtime_value; 		///���������߶�ʵʱֵ����λΪ1/10mm
	uint16_t	oil_average_value;	///���������߶�ƽ��ʵʱֵ����λΪ1/10mm
	
	u8			oil_YH_workstate; 	 //  0:  ��ʾ�����쳣	1: ��ʾ��������  2:�ܹ���⵽��������������
	
	u32 		oil_YH_Abnormal_counter;	 //   ������ʻ�����У� ��ʵʱ�ͺ���ֵΪ 0  ��
										//	 ����300 �Σ��������300 ���ж��ͺĹ���
	u32 		oil_YH_0_value_cacheCounter;	//	�ٶ�Ϊ0 ������ ���ٶ�Ϊ0 ��������30 �� ��������������� 								   
	u32         oil_YH_no_data_Counter;  //  û�����ݼ�����
}YH;


extern  YH   Oil;

// Lora related  
extern u8	 Lora_config_CMD[6] ;
extern LORA_STATE   LORA_RUN;

extern  struct rt_device  Device_UsrSerial;

extern void  u3_RxHandler(unsigned char rx_data);
extern void  Device_UsrSerial_regist(void );
extern void Oil_Sensor_Connect_Checking(void);
extern void LORA_Rx_Process(void); 
extern void lora_mode(u8 value);
extern void LoRA_TX_Process(void);



#endif
