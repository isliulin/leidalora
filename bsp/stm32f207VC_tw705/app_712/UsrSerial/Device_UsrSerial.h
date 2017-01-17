#ifndef   _CAN2_DEV
#define    _CAN2_DEV
#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//数字转换成字符串
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


#define      LORA_SEND_MODE           1     //  1  : 定向发送模式     0:  透传模式


typedef struct  _LORA_STATE
{
  u8    SD_Enable  ;    //  发送标志位
  u8    HandsendFlag;   //  手动发送标志位
  u16   Time_counter;   //  计数器  
  //  Note	:  当前应用只设置地址和信道号,HEAD 采用C0  (掉电保存)	1A 传输速率  C4:定向发送方式
  //											 HEAD	ADDH  ADDL	Baud  CHNL	  TX_MODE
  //                                                                              {0xC0, 0x12, 0x34, 0x1A, 0x17, 0xC4};
  u8   Config_CMD[6] ; //  设置命令字段
  u8   TX_buff[256];   //  发送字符串
  u8   Tx_Disp[256];
  u32   MSG_ID;   // 发送消息ID
} LORA_STATE;





typedef  struct   _YH
{    
	uint32_t	oil_value;			///邮箱油量，单位为1/10升
	uint16_t	oil_realtime_value; 		///邮箱油量高度实时值，单位为1/10mm
	uint16_t	oil_average_value;	///邮箱油量高度平均实时值，单位为1/10mm
	
	u8			oil_YH_workstate; 	 //  0:  表示工作异常	1: 表示工作正常  2:能够检测到，但传感器故障
	
	u32 		oil_YH_Abnormal_counter;	 //   车辆行驶过程中， 但实时油耗数值为 0  ，
										//	 计算300 次，如果大于300 则判断油耗故障
	u32 		oil_YH_0_value_cacheCounter;	//	速度为0 ，缓冲 ，速度为0 ，且连续30 次 ，才清除避免误判 								   
	u32         oil_YH_no_data_Counter;  //  没有数据计数器
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
