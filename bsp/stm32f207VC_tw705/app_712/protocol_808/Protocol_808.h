/*
     Protocol 808 .h
*/
#ifndef  _PROTOCOL808
#define   _PROTOCOL808

#include <rthw.h>
#include <rtthread.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>
#include "App_moduleConfig.h"
#include "gps.h"
#include "DF_Oper.h"

#define  AVRG15MIN          // 使能记录 停车前15 分钟平均速度


#define  CURREN_LIM_Dur        1

// ----   Media  Trans state ---
#define   MEDIA
#define  enable                1
#define  disable               0
#define  other                  2
#define  transfer             3


//---------------------------
#define  Max_PKGRecNum_0704     5   // 0704 单包最大记录数目   58 *16 =986  单包不超过1024  
#define  Limit_packek_Num        3    // 限制每包上报间隔

//------------  GPS function------
#define  INIT         1
#define  PROCESS      0

#define  K_adjust_Duration       30                //  校验K值所需要时间   


// -------  ISP  Address   -------------
#define    ISP_APP_Addr                        0x6400    //  512*50  Page
#define    ISP_RAM_Addr                       0x1400   // 512*10 Page

//--------ISP Status Byte  ------------
#define    ISP_BYTE_StartValue            0xF1     // 远程下载开始字节
#define    ISP_BYTE_CrcPass               0xE1     //  接收完数据包后，改成这个数值，表示校验通过
#define    ISP_BYTE_TypeNotmatch          0xC1     //   校验通过，但类型不匹配
#define    ISP_BYTE_Rdy2Update            0x01     //   校验通过类型匹配，等待更新


//------- 通话状态---------
#define CallState_Idle 		        0
#define CallState_Ring 		        1
#define CallState_Connected	        2
#define CallState_Hangup		    4
#define CallState_rdytoDialLis      5
#define CallState_Dialing           8   //主叫拨号


/*
          APP   级别 的应用
*/
#define    LORA_RELAYSTAION      0
#define    LORA_RADRCHECK        1
#define    LORA_ENDPLAY          2
#define    LORA_HANDLE_DEV       3 


typedef struct  _SYSConfig           //  name:  config
{
    u16     Version_ID ;   //  系统版本IDE
    u16     LORA_Local_ADDRESS;  //   LORA address 
    u16     LORA_Baud;     //  Baud 
    u8      LORA_Local_Channel;  //   LORA channel
    u16     LORA_dest1_ADDRESS;  //   dest1 address 
    u8      LORA_dest1_Channel;  //   dext1 channel
    u8      LORA_PointDesc[100]; // LORA  description      
    u8      LORA_SYS_Channel;  //   LORA  系统频点
    u8      LORA_TYPE;     //  节点类型           0:  中继   1: 雷达监测点   2:  道口接收  3 :手持终端
    u8      LORA_DIRECTION; //    0      1      2      3        4      5     6       7       
                            //   EE    SS   WW    NN   ES    EN    WS   WN
    u8      RTC_updated;  //    RTC 是否被校准                        
} SYS_CONF;



extern ALIGN(RT_ALIGN_SIZE) u8          GPRS_info[1400];
extern u16         GPRS_infoWr_Tx;


//------ phone
extern u8       CallState; // 通话状态


//==================================================================================================
// 第二部分 :   以下是外部传感器状态监测
//==================================================================================================
/*
     -----------------------------
     2.1   和协议相关的功能函数
     -----------------------------
*/
extern int IP_Str(char *buf, u32 IP);
extern void strtrim(u8 *s, u8 c);
extern int str2ip(char *buf, u8 *ip);
extern void  Enable_Relay(void);
extern void  Disable_Relay(void);


/*
     -----------------------------
    2.4  不同协议状态寄存器变化
     -----------------------------
*/

extern void StatusReg_WARN_Enable(void);
extern void StatusReg_WARN_Clear(void);
extern void StatusReg_ACC_ON(void);
extern void StatusReg_ACC_OFF(void);
extern void StatusReg_POWER_CUT(void);
extern void StatusReg_POWER_NORMAL(void);
extern void StatusReg_GPS_A(void);
extern void StatusReg_GPS_V(void);
extern void StatusReg_SPD_WARN(void);
extern void StatusReg_SPD_NORMAL(void);
extern void StatusReg_Relay_Cut(void);
extern void StatusReg_Relay_Normal(void);
extern void StatusReg_Default(void);









//==================================================================================================
// 第三部分 :   以下是GPRS无线传输相关协议
//==================================================================================================

extern  u8  Do_SendGPSReport_GPRS(void);
extern void  Save_GPS(void);

extern void  ISP_file_Check(void);
extern unsigned short int  File_CRC_Get(void);
extern u16   Instr_2_GBK(u8 *SrcINstr, u16 Inlen, u8 *DstOutstr );


extern void delay_us(u16 j);
extern void delay_ms(u16 j);


#if 0
extern void  Stuff_ISP_ack(void);      // ISP
extern void  Stuff_ISPinfo_ack(void);      // ISP
#endif

extern  void  Media_Start_Init( u8  MdType , u8  MdCodeType);
extern  void  Media_Clear_State(void);
//extern  void  Media_Timer(void);
extern  void  Media_RSdMode_Timer(void);
extern  void  Media_Timer_Service(void);
extern  void  Meida_Trans_Exception(void);

extern void   Photo_send_start(u16 Numpic);
extern  void  DataTrans_Init(void);

#ifdef REC_VOICE_ENABLE
extern u8     Sound_send_start(void);
#endif

extern void  TCP_RX_Process(u8  LinkNum);
extern u16    AsciiToGb(u8 *dec, u8 InstrLen, u8 *scr);
extern void  Time2BCD(u8 *dest);



extern void SpeedWarnJudge(void);
extern void Process_GPRSIN_DeviceData(u8 *instr, u16  infolen);


extern void  Sleep_Mode_ConfigEnter(void);
extern void  Sleep_Mode_ConfigExit(void);

//extern u16   WaveFile_EncodeHeader(u32 inFilesize ,u8* DestStr);
extern void  CycleRail_Judge(u8 *LatiStr, u8 *LongiStr);
extern void  RectangleRail_Judge(u8 *LatiStr, u8 *LongiStr);
extern u8    Save_MediaIndex( u8 type, u8 *name, u8 ID, u8 Evencode);
extern void  vin_set(u8 *instr);
extern void Tired_Check(void);
extern void OutPrint_HEX(u8 *Descrip, u8 *instr, u16 inlen);
//extern void  Sound_SaveStart(void);
//extern void  Sound_SaveEnd(void);
extern void    DoorCameraInit(void);
extern void    MSG_BroadCast_Read(void);
extern u8      Time_FastJudge(void);
extern void   CAN_struct_init(void);
extern void   CAN_send_timer(void);
extern void   buzzer_onoff(u8 in);

#endif


extern void  Lora_Gprs_Send(u8 *instr, u16 inlen);


