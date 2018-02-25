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




#define   LORA_MD1_PORT     GPIOE
#define   LORA_MD1_PIN      GPIO_Pin_14  

#define   LORA_MD0_PORT      GPIOE
#define   LORA_MD0_PIN       GPIO_Pin_13  


#define      LORA_MD1_HIGH       GPIO_SetBits(LORA_MD1_PORT,LORA_MD1_PIN)
#define      LORA_MD1_LOW        GPIO_ResetBits(LORA_MD1_PORT,LORA_MD1_PIN)

#define      LORA_MD0_HIGH       GPIO_SetBits(LORA_MD0_PORT,GPIO_Pin_13)
#define      LORA_MD0_LOW        GPIO_ResetBits(LORA_MD0_PORT,GPIO_Pin_13) 


typedef struct  _LORA_STATE
{
  u8    SD_Enable  ;    //  发送标志位
  u16   Time_counter;   //  计数器  
  //  Note	:  当前应用只设置地址和信道号,HEAD 采用C0  (掉电保存)	1A 传输速率  C4:定向发送方式
  //											 HEAD	ADDH  ADDL	Baud  CHNL	  TX_MODE
  //                                                                              {0xC0, 0x12, 0x34, 0x1A, 0x17, 0xC4};
  u8    Tx_Disp[100];
  u32   TX_MSG_ID;   // 发送消息ID
  u32   RX_MSG_ID;// 接收消息ID
  u8    ComeDirectStr[50]; // 车辆来向
  u8    Come_state; //  车辆来状态
  u8    ComeSPD;

  //       巡检标志位
  u8    SD_check_Enable;  //  发送巡检消息标志位

  //   ---  TTS play   每个消息波3 遍 ，来新消息即刻停止 以前播放
  u8     Played_times;  // 当前播放的次数
  u8     Play_str[200];

  //  ----巡检反馈---
  u8  ACK_INFO[4][20];    // 反馈消息 最多4 条
  u8  ACK_index;          // 反馈下标

  u8  SD_waitACK_Flag;   //  发送后等待返回状态位        1: 发送后等待    2: 等待过后没反馈  0: 常态
  


u8	test_flag;
u8	test_counter;  
u8	test_send_flag; 


  
} LORA_STATE;






// Lora related  
extern u8	 Lora_config_CMD[6] ;
extern LORA_STATE   LORA_RUN;

extern  struct rt_device  Device_UsrSerial;

extern void  LORA_RxHandler(unsigned char rx_data);
extern void  Device_UsrSerial_regist(void );
extern void LORA_Rx_Process(void); 
extern void lora_mode(u8 value);
extern void LoRA_TX_Process(void);
extern void Lora_MD_PINS_INIT(void);
extern void  loramodule_get(void);
extern void  loramodule_set(u16 Addr,u8 channel);
extern void  lora_send(u32 ID,u8  SPD);
extern void  Lora_channel(u8  channel);
extern void  Lora_TTS_play_Process(void);

//---- zhuigeer----
extern void test_ack_timer(void);


#endif
