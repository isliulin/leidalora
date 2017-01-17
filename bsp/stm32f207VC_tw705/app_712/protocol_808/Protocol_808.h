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


//-----------------行车记录仪  附录A 命令字 --------------------
// 1. 上载命令字
#define  A_Up_DrvInfo          0x01        //  当前驾驶人信息
#define  A_Up_RTC              0x02        //  采集记录仪的实时时钟
#define  A_Up_Dist             0x03        //  采集里程
#define  A_Up_PLUS             0x04        //  采集记录仪特征系数
#define  A_Up_AvrgMin          0x05        //  每分钟平均速度记录
#define  A_Up_VechInfo         0x06        //  车辆信息
#define  A_Up_Doubt            0x07        //  事故疑点数据
#define  A_Up_N2daysDist       0x08        //  采集最近2个日历天的累计行驶里程
#define  A_Up_N2daysSpd        0x09        //  采集最近2个日历天内的行驶速度数据 
#define  A_Up_Tired            0x11        //  疲劳驾驶记录   

// 2. 下传命令
#define  A_Dn_DrvInfo          0x81        //  设置驾驶员信息
#define  A_Dn_VehicleInfo      0x82        //  设置车辆信息
#define  A_Dn_RTC              0xC2        //  设置记录仪时间
#define  A_Dn_Plus             0xC3        //  设置速度脉冲系数

// 3.  传输方式
#define   Trans_Serial         0           //  串口方式传输
#define   Trans_WireLess       1           //  无线传输


// 4. 808 协议   消息ID
#define   MSG_0x0001           0x0001      // 终端通用应答
#define   MSG_0x8001           0x8001      // 平台通用应答
#define   MSG_0x0002           0x0002      // 终端心跳 
#define   MSG_0x0003           0x0003      // 终端注销
#define   MSG_0x0100           0x0100      // 终端注册
#define   MSG_0x8100           0x8100      // 终端注册应答
#define   MSG_0x0101           0x0101      // 终端注销
#define   MSG_0x0102           0x0102      // 终端鉴权
#define   MSG_0x8103           0x8103      // 设置终端参数 
#define   MSG_0x8104           0x8104      // 查询终端参数
#define   MSG_0x0104           0x0104      // 查询终端参数应答
#define   MSG_0x8105           0x8105      // 终端控制
#define   MSG_0x0200           0x0200      // 位置信息汇报 
#define   MSG_0x8201           0x8201      // 位置信息查询
#define   MSG_0x0201           0x0201      // 位置信息查询应答 
#define   MSG_0x8202           0x8202      // 临时位置跟踪控制
#define   MSG_0x8300           0x8300      // 文本信息下发
#define   MSG_0x8301           0x8301      // ---事件设置
#define   MSG_0x0301           0x0301      // ---事件报告
#define   MSG_0x8302           0x8302      // ---提问下发
#define   MSG_0x0302           0x0302      // ---提问应答
#define   MSG_0x8303           0x8303      // ---信息点播菜单设置
#define   MSG_0x0303           0x0303      // ---信息点播 取消
#define   MSG_0x8304           0x8304      // ---信息服务
#define   MSG_0x8400           0x8400      // ---电话回拨
#define   MSG_0x8401           0x8401      // ---设置电话本
#define   MSG_0x8500           0x8500      // 车辆控制
#define   MSG_0x0500           0x0500      // 车俩控制应答
#define   MSG_0x8600           0x8600      // 设置圆形区域
#define   MSG_0x8601           0x8601      // 删除圆形区域
#define   MSG_0x8602           0x8602      // 设置矩形区域
#define   MSG_0x8603           0x8603      // 删除矩形区域
#define   MSG_0x8604           0x8604      // 设置多边形区域
#define   MSG_0x8605           0x8605      // 删除多边形区域
#define   MSG_0x8606           0x8606      // 设置路线
#define   MSG_0x8607           0x8607      // 删除路线
#define   MSG_0x8700           0x8700      // 行车记录仪数据采集命令
#define   MSG_0x0700           0x0700      // 行驶记录仪数据上传
#define   MSG_0x8701           0x8701      // 行驶记录仪但是下传
#define   MSG_0x0701           0x0701      // ---电子运单上报
#define   MSG_0x0702           0x0702      // 驾驶员身份信息采集上报
#define   MSG_0x0107           0x0107      //  BD --终端属性查询
#define   MSG_0x0108           0x0108      //  BD--终端升级结果通知
#define   MSG_0x0704           0x0704      //  BD--定位数据批量上传
#define   MSG_0x0705           0x0705      //  BD-- CAN 总线数据上传
#define   MSG_0x0805           0x0805      //  BD-- 摄像头立即拍摄命令应答
#define   MSG_0x0800           0x0800      // ---多媒体事件信息上传
#define   MSG_0x0801           0x0801      // ---多媒体数据上传
#define   MSG_0x8800           0x8800      // ---多媒体数据上传应答
#define   MSG_0x8801           0x8801      // 摄像头立即拍照命令
#define   MSG_0x8802           0x8802      // 存储多媒体数据检索
#define   MSG_0x0802           0x0802      // 存储多媒体数据检索应答
#define   MSG_0x0805           0x0805     //  拍照上传立即应答 
#define   MSG_0x8803           0x8803      // 存储多媒体数据上传
#define   MSG_0x8804           0x8804      // 录音开始命令 
#define   MSG_0x8900           0x8900      // 数据下行透传
#define   MSG_0x0900           0x0900      // 数据上行透传
#define   MSG_0x0901           0x0901      // 数据压缩上报
#define   MSG_0x8A00           0x8A00      // 平台RSA公钥
#define   MSG_0x0A00           0x0A00      // 终端RSA公钥

//--------  发送包的方式  ------
#define   Packet_Normal         0        //  正常包     
#define   Packet_Divide         1        //  分包

#define   SpxBuf_Size       512
#define   SpxGet_Size       100



typedef struct _A_AckFlag
{
    // 1. 上载命令字
    u8  A_Flag__Up_Ver_00H             ;// 0x00   1     //  记录仪执行标准版本号
    u8  A_Flag_Up_DrvInfo_01H          ;// 0x01   2     //  当前驾驶人信息
    u8  A_Flag_Up_RTC_02H              ;// 0x02   3     //  采集记录仪的实时时钟
    u8  A_Flag_Up_Dist_03H             ;//0x03    4    //  采集里程
    u8  A_Flag_Up_PLUS_04H             ;//0x04    5    //  采集记录仪速度脉冲系数
    u8  A_Flag_Up_VechInfo_06H         ;//0x06    6    //  车辆信息
    u8  A_Flag_Up_SetInfo_08H          ;//0x08    7    //  状态配置信息
    u8  A_Flag_Up_DevID_16H            ;//0x16    8    //  记录仪唯一ID
    u8  A_Flag_Up_AvrgSec_09H          ;//0x09    9    //  每秒钟平均速度记录
    u8  A_Flag_Up_AvrgMin_05H          ;//0x05    10    //  每分钟平均速度记录
    u8  A_Flag_Up_Posit_13H            ;//0x13    11    //  指定位置信息记录
    u8  A_Flag_Up_Doubt_07H            ;//0x07    12    //  事故疑点数据
    u8  A_Flag_Up_Tired_11H            ;//0x11    13    //  疲劳驾驶记录
    u8  A_Flag_Up_LogIn_10H            ;//0x10    14    //  登入登出记录
    u8  A_Flag_Up_Powercut_14H         ;//0x14    15    //  外部供电记录
    u8  A_Flag_Up_SetMdfy_15H          ;//0x15    16    //  参数修改记录

    // 2. 下传命令
    u8  A_Flag_Dn_DrvInfo_82H          ;//0x82    17    //  设置车辆信息
    u8  A_Flag_Dn_SetupDate_83H        ;//0x83    18    //  设置初装日期
    u8  A_Flag_Dn_Satus_84H            ;//0x84    19    //  设置状态量信息
    u8  A_Flag_Dn_RTC_C2H              ;//0xC2    20    //  设置记录仪时间
    u8  A_Flag_Dn_Plus_C3H             ;//0xC3    21    //  设置速度脉冲系数
} A_AckFlag;

typedef struct _GPRMC_PRO
{
    void (*Time)(u8 *tmpinfo, u8 hour, u8 min , u8 sec);
    void (*Status)(u8 *tmpinfo);
    void (*Latitude)(u8 *tmpinfo);
    void (*Latitude_NS)(u8 *tmpinfo);  // S 南纬  N  北纬
    void (*Longitude)(u8 *tmpinfo);
    void (*Longitude_WE)(u8 *tmpinfo); // E 东经  W  西经
    void (*Speed)(u8 *tmpinfo, u8 Invalue, u8 Point);
    void (*Direction)(u8 *tmpinfo, u8 Invalue, u8 Point);
    void (*Date)(u8 *tmpinfo, u8 fDateModify, u8 hour, u8 min , u8 sec);

} GPRMC_PRO;

typedef struct _Position
{
    u8  latitude_BgEnd[4];
    u8  longitude_BgEnd[4];
} POSIT;




typedef struct _TCP_ACKFlag
{
    u8  f_CentreCMDack_0001H;
    u8  f_CentreCMDack_resualt;
    u8  f_CurrentPosition_0201H;
    u8  f_CurrentEventACK_0301H; //  事件报告
    u8  f_MsgBroadCast_0303H;//  信息点播/ 取消
    u8  f_MediaIndexACK_0802H; //  多媒体应答      0  idle  1  查询图片   2  查询音频  3 查询视频
    u8  f_QueryEventCode;  //  0  平台下发命令   1 定时动作   2  抢劫报警触发 3  碰撞触发
    u8  f_SettingPram_0104H;
    u8  f_DriverInfoSD_0702H; //   驾驶员 身份信息采集上报
    u8  f_Worklist_SD_0701H; //   电子运单上传
    //  -----  北斗扩展----
    u8  f_BD_Extend_7F02H; // 北斗信息查询应答
    u8  f_BD_Extend_7F00H;// 扩展终端上发指令
    u8  f_BD_DeviceAttribute_8107; //  中心查找终端属性
    u8  f_BD_BatchTrans_0704H;   //  定位数据批量上传
    u8  f_BD_CentreTakeAck_0805H; // 中心拍照命令应答
    u8  f_BD_ISPResualt_0108H;   //    终端升级结果     1  :成功    2: 失败      3:取消

} TCP_ACKFlag;


typedef struct _NandSVflag
{
    u8  Doubt_SaveFlag;      // 存储事故疑点
    u8  MintPosit_SaveFlag;  // 存储每分钟位置
    u8  PowerCut_SaveFlag;   // 存储断电记录
    u8  Log_SaveFlag;        // 存储登录记录
    u8  Setting_SaveFlag;    // 存储参数修改记录


} NANDSVFlag;

typedef struct _Stdver
{
    u8 stdverStr[14];   //记录仪执行的国标版本
    u8 MdfyID;          // 修改单号

} STD_VER;

typedef struct _DriveInfo
{
    u8 Effective_Date[3];  // 驾驶证有效期
    u8 DriverCard_ID[19];  // 驾驶员驾驶证号码 18位
    u8 DriveName[22];	   // 驾驶员 姓名21
    u8 Drv_CareerID[21];  // 驾驶员从业资格证20
    u8 Comfirm_agentID[41]; // 发证机构名称 40
} DRV_INFO;

typedef struct _VechInfo
{
    u8     Vech_VIN[18];        // 车辆VIN号17
    u8     Vech_Num[13];	     // 车牌号12
    u8     Vech_Type[13];       // 车辆类型 12
    u16    Dev_ProvinceID;      // 车辆所在省ID
    u16    Dev_CityID;          // 车辆所在市ID
    u8     Dev_Color;           // 车牌颜色           // JT415    1  蓝 2 黄 3 黑 4 白 9其他
    u8     loginpassword_flag;  //  界面输入标志位    0 :default    1:  longin ok
    u8     Link_Frist_Mode;		 //   首次连接模式		  0  : dnsr first	  1: mainlink  first
    u8     Vech_Type_Mark;      //    1:两客一危    2:货运
    u8     ProType[5];
} VechINFO;

//--------- 终端属性---------
typedef struct  _PRODUCT_ATTRIBUTE
{
    u16  _1_DevType; //   终端类型
    u8   _2_ProducterID[5];// 制造商ID
    u8   _3_Dev_TYPENUM[20];//终端型号
    u8   _4_Dev_ID[7];//终端ID
    u8   _5_Sim_ICCID[10];//终端SIM卡ICCID
    u8   _6_HardwareVer_Len;//硬件版本号长度
    u8   _7_HardwareVer[20]; // 硬件版本号
    u8   _8_SoftwareVer_len;// 软件版本号长度
    u8  _9_SoftwareVer[20];//软件版本号
    u8 _10_FirmWareVer_len;// 固件版本号长度
    u8 _11_FirmWare[20];// 固件版本号
    u8 _12_GNSSAttribute; // 定位模块属性
    u8 _13_ComModuleAttribute;// 通信模块属性
} PRODUCT_ATTRIBUTE;

//-------- 北斗 扩展  -------------
typedef struct _BD_EXTEND
{
    // 1. ----  车台设置相关 ----
    u32	  Termi_Type;	 // 终端类型
    u32	  Software_Ver; //	软件版本
    u32	  GNSS_Attribute;	// BD  模块属性
    u32	  GSMmodule_Attribute; // GSM 模块属性
    u32	  Device_Attribute;   //  终端属性

    //  2.  北斗设置
    u8	 GNSS_Mode; 	//	北斗模块通信模式
    u8	 GNSS_Baud; 	//	北斗模块 通信波特率  表示
    u32   GNSS_Baud_Value;//   北斗模块数值
    u8	   BD_OutputFreq;  // 北斗模块输出更新率设置
    u32   BD_SampleFrea; //  北斗模块采集 NMEA	数据频率
    //-----  2013 BD add -------------
    u8	   BD_OriginalDataTransMode;	 //  GNSS 模块详细定位信息上传方式
    u8	   BD_OriginalDataTransSettings; //  GNSS 模块详细数据上传设置01: 单位为秒	02 : 单位为米 03: 单位为秒


    // 3.	CAN   相关设置
    u32   CAN_1_Mode;	 //  CAN  模式	01:还回模式  10  : 普通模式   11:  静默模式
    u32   CAN_1_ID;
    u8	  CAN_1_Type;
    u8	  CAN_1_SampleDuration;  //  单位秒
    u8	  CAN_1_TransDuration;	//	单位 秒

    u32   CAN_2_Mode;	//	 CAN  模式
    u32   CAN_2_ID;
    u8	   CAN_2_Type;
    u8	   CAN_2_SampleDuration;  //  单位秒
    u8	   CAN_2_TransDuration;  //  单位 秒
    u32   Collision_Check;	   //  bit 31 : 开启关闭碰撞  b7-b0  碰撞时间门限 b8-b15  碰撞加速度门限

    u8	   CloseAllcan;

    //   位置附加信息
    // 1. 信号强度
    u16  FJ_SignalValue;	//	信号强度   高字节 0 ；低字节  高4为 X2 gprs强度 ，低4位 卫星颗数
    //  2. 自定义状态机模拟量上传
    u8	FJ_IO_1;
    u8	 FJ_IO_2;
    u16	AD_0;  //  2 Byte
    u16	AD_1; //   2 Byte

    u8  Extent_IO_status;  // 新北斗协议 IO 状态


    //	  围栏相关判断
    u8	Close_CommunicateFlag;	//	进区域 关闭通信
    u8	Trans_GNSS_Flag;	//	进区域采集GNSS



} BD_EXTEND;



//-------

//-------车台注册相关 ------------
typedef struct _DevRegst
{
    u8  Sd_counter; //  使能发送间隔计数器
    u8  Enable_sd;  //  使能发送 注册信息
    u8  DeRegst_sd; //  注销车台
    u8  Sd_add;        // 注册重发次数
} DevRegst;

//------- 车台鉴权相关 ----------
typedef struct _DevLOGIN
{
    u8  Operate_enable;  // 使能开始操作鉴权操作   1 :enable  0: idle
    u8  Sd_counter;      // 定时发送计数器
    u8  Sd_times;        // 发送次数计数器
    u8  Enable_sd;       // 使能发送鉴权信息
    u8  Sd_add;           //  鉴权发送次数
} DevLOGIN;
//------- 超速报警 -------
typedef struct SPDEXP
{
    u8  excd_status;
    u16 current_maxSpd;    //  超速报警下 当前的最大速度
    u32 dur_seconds;       //  超速状态计数器    单位: s
    u8  speed_flag;        //  超速报警触发后先立即上报一包标志位
    u8  ex_startTime[6];
    u8  ex_endTime[6];

	u8   PlayState; //  播放状态
	u32  PlayCounter;  // 
	
} SPD_EXP;
//------- 疲劳驾驶门限 ----
typedef struct _TIRED_DOOR
{
    //  单位:  s
    u32  Door_DrvKeepingSec;       // 连续驾驶时间
    u32  Door_DayAccumlateDrvSec;  // 当天累加驾驶时间
    u32  Door_MinSleepSec;         // 最小休息时间
    u32  Door_MaxParkingSec;       // 最长停车时间
    u32  Parking_currentcnt;       // 停车实时计数器
} TIRED_DOOR;
//-------- 上报间隔相关  ---------
typedef struct _SEND_DUR
{
    //  单位: s
    u32  Heart_Dur;     //  终端心跳包发送间隔
    u16  Heart_SDCnter; // 心跳包计数器
    u8   Heart_SDFlag;   // 心跳包发送标志位
    //------------------
    u32  TCP_ACK_Dur;   //  TCP 消息应答超时
    u16  TCP_ACK_DurCnter;// TCP 计数器
    u8   TCP_SD_state;    // 发送一包后置1.
    //------------------
    u32  TCP_ReSD_Num;  //  TCP 重传次数
    u16  TCP_ReSD_cnter; // 发送计数器
    //------------------
    u32  UDP_ACK_Dur;   // UDP消息应答超时
    u32  UDP_ReSD_Num;  // UDP 重传次数

    u32  NoDrvLogin_Dur;  //  驾驶员未登录汇报时间间隔
    // u16  NoLogin_SDcnter;   //  发送计时器
    //------------------
    u32  Sleep_Dur;     //  休眠时候上报的时间间隔
    //u16  Sleep_SDcnter;   //  发送计数器
    //------------------
    u32  Emegence_Dur;  //  紧急报警时上报的时间间隔
    //u16  Emegence_SDcnter;
    //------------------
    u32  Default_Dur;   //  缺省时间汇报间隔

    u32  SD_Delta_maxAngle; // 拐点补传角度   < 180

    u16  IllgleMovo_disttance; // 非法移动阈值   -- 补充协议要求添加
} SEND_DUR;
// ----------- 定距离上报相关  ---------
typedef struct _SEND_DIST
{
    //   单位: m
    u32  Defalut_DistDelta; // 缺省定距离上报
    u32  NoDrvLogin_Dist;   // 驾驶员未登录 上博距离
    u32  Sleep_Dist;        // 休眠情况下 上报距离
    u32  Emergen_Dist;      // 紧急报警时 上报距离
} SEND_DIST;

//----------- 上报模式  ---------------
typedef struct _SEND_MODE
{

    u8 Send_strategy;
    u8 DUR_TOTALMODE;     // -----   定时上报方式
    u8 Dur_DefaultMode;
    u8 Dur_SleepMode;
    u8 Dur_NologinMode;
    u8 Dur_EmegencMode;

    u8 DIST_TOTALMODE;    // ------   定距上报方式
    u8 Dist_DefaultMode;
    u8 Dist_NoLoginMode;
    u8 Dist_SleepMode;
    u8 Dist_EmgenceMode;

} SEND_MODE;

//------- 实时位置跟踪 ----
typedef struct _REALTIME_LOCK
{
    u8  Lock_state;   //  实时跟踪启动标志位
    u16 Lock_Dur;     //  限定的上报时间间隔
    u32 Lock_KeepDur; //  实时上报持续时间
    u32 Lock_KeepCnter;

} REALTIME_LOCK;

//------- 文本信息 --------
typedef struct _TEXT_INFO
{
    u8  TEXT_FLAG;          //  文本标志
    u8  TEXT_SD_FLAG;       // 发送标志位
    u8  TEXT_Content[DFBakSize];  // 文本内容
} TEXT_INFO;

//----- 信息 ----
typedef struct _MSG_TEXT
{
    u8   TEXT_mOld;     //  最新的一条信息  写为1代表是最新的一条信息
    u8   TEXT_TYPE;     //  信息类型   1-8  中第几条
    u8   TEXT_LEN;      //  信息长度
    u8   TEXT_STR[150]; //  信息内容
} MSG_TEXT;

//-----  提问 ------
typedef struct _CENTER_ASK
{
    u8  ASK_SdFlag; //  标志位           发给 TTS  1  ；   TTS 回来  2
    u16 ASK_floatID; // 提问流水号
    u8  ASK_infolen;// 信息长度
    u8  ASK_answerID;    // 回复ID
    u8  ASK_info[60];//  信息内容
    u8  ASK_answer[140];  // 候选答案
    u8  ASK_disp_Enable;// 小屏显示使能
} CENTRE_ASK;


//------- 车辆控制  --------
typedef struct _VEHICLE_CONTROL
{
    u8   Control_Flag;  // 控制 标志
    u16  CMD_FloatID;   // 下发应答的消息流水号
    u8   ACK_SD_Flag;   // 回复发送标志位

} VEHICLE_CONTROL;

//--------  行车记录仪相关  -----
typedef struct  _RECODER
{
    u16  Float_ID;	   //  命令流水号
    u8	 CMD;	  //  数据采集
    u8	 SD_Data_Flag; //  发送返回数返回标志
    u8	 CountStep;  //  发送数据需要一步一步发送
    u32   timer;
    //--------- add on	5-4
    u8	 Devide_Flag;//  需要分包上传标志位
    u16  Total_pkt_num;   // 分包总包数
    u16  Current_pkt_num; // 当前发送包数 从 1	开始
    u16  Read_indexNum;  // 读取记录数目 ，不是包数，(有时读取记录数= 报数*子包数)
    u8	  fcs;
    u8	Error;//	0 idle	 1:   采集错误	 2:  设置错误

    //---------  记录仪仪采集----------------------
    u8   Get_withDateFlag;  //  根据时间采集
    u8   Get_startDate[6];  // 采取起始时间
    u8   Get_endDate[6];    //  采集结束时间
    u16  Get_recNum;      //    获取的条目数

} RECODER;
//------  Camera  --------
typedef struct _CAMERA
{
    u8 Channel_ID;     //  拍照通道
    u8 Operate_state;  //  拍照后处理状态    0 : 保存   1:  实时上传



} CAMERA;
//---------- 多媒体  ----------
typedef struct _MULTIMEDIA
{
    u32  Media_ID;           //   多媒体数据ID
    u8   Media_Type;         //   0:   图像    1 : 音频    2:  视频
    u8   Media_CodeType;     //   编码格式  0 : JPEG  1:TIF  2:MP3  3:WAV  4: WMV
    u8   Event_Code;         //   事件编码  0: 平台下发指令  1: 定时动作  2 : 抢劫报警触发 3: 碰撞侧翻报警触发 其他保留
    u8   Media_Channel;      //   通道ID
    //----------------------
    u8   SD_Eventstate;          // 发送事件信息上传状态    0 表示空闲   1  表示处于发送状态
    u8   SD_media_Flag;     // 发送没提事件信息标志位
    u8   SD_Data_Flag;      // 发送数据标志位
    u8   SD_timer;          // 发送定时器
    u8   MaxSd_counter;   // 最大发送次数
    u8   Media_transmittingFlag;  // 多媒体传输数据状态  1: 多媒体传输前发送1包定位信息    2 :多媒体数据传输中  0:  未进行多媒体数据传输
    u16  Media_totalPacketNum;    // 多媒体总包数
    u16  Media_currentPacketNum;  // 多媒体当前报数
    //----------------------
    u8   RSD_State;     //  重传状态   0 : 重传没有启用   1 :  重传开始    2  : 表示顺序传完但是还没收到中心的重传命令
    u8   RSD_Timer;     //  传状态下的计数器
    u8   RSD_Reader;    //  重传计数器当前数值
    u8   RSD_total;     //  重传选项数目


    u16	Media_ReSdList[125]; //  多媒体重传消息列表
} MULTIMEDIA;
//------  Voice Record 录音相关 ----
typedef struct _VOICE_RECODE
{
    u8   Operate_CMD; //  操作命令  0: 停止录音    1: 开始录音
    u16  Voice_Dur;   //  录音时间， 0 表示一直录
    u8   Save_Flag;   //  0:  实时上传   1  保存
    u8   SampleRate;  //  音频采样率   0:8K   1: 11k  2:23k  3: 32K  , 其他保留

} VOICE_RECODE;

//----------   DataTrans  数据透传  -----------
typedef struct _DATATRANS
{
    u8 TYPE;             // 透传消息类型
    u8 Data_RxLen;       // 接收信息长度
    u8 Data_TxLen;       // 发送信息长度
    u8 DataRx[10];      // 透传信息内容接收
    u8 Data_Tx[20];     // 透传信息内容发送
    u8 Tx_Wr;

} DATATRANS;
//------  进出区域报警 -------------------
typedef struct _INOUT
{
    u8  TYPE; //   1 圆形区域 2 矩形区域 3 多边形区域 4 路线
    u32 ID;   //    区域路线ID
    u8  InOutState; //  0 进  1  出
} INOUT;
//-------  多媒体索引 -------------------
typedef struct _MEDIA_INDEX
{
    u8  Effective_Flag; // 存储数目
    u32  MediaID;  // 多媒体ID
    u8  Type;     // 类型    0 图像   1  音频   2  视频
    u8  ID  ;      // 通道
    u8  EventCode; // 事件编码    0  平台下发命令   1 定时动作   2  抢劫报警触发 3  碰撞触发
    u8  FileName[12];  // 文件名
    u8  PosInfo[28];//位置信息

} MEDIA_INDEX;
//----------- 车门开关拍照相关 ------------
typedef struct _DoorCamra
{
    u8	  currentState;  // 当前状态
    u8	  BakState;   // 存储状态
} DOORCamera;


typedef struct   TGPSInfo_GPRS
{
    //---------------  河北天地通协议  ---------------------------
    u8		Date[3];	// 年月日(HEX)
    u8		Time[3];	// 十分秒(HEX)
    u8		Latitude[4];    //纬度	  N/S
    u8		Longitude[4];   //经度	 W/E
} T_GPS_Info_GPRS;

typedef struct _DetachPKG
{
    u16  Original_floatID;  //  原始消息流水号
    u8    NumOf_Resend;   //  重传包序号
    u16  List_Resend[50];  //重传包ID 列表

} DETACH_PKG;

typedef struct  _SET_QRY
{
    u8    Num_pram;//   参数个数
    u32  List_pram[90];//  cansh ID
} SET_QRY;

typedef struct _HUMAN_CONFIRM_WARN
{
    u16 Warn_FloatID;// 报警消息流水号
    u32 ConfirmWarnType; // 人工确认报警类型

} HUMAN_CONFIRM_WARN;

//  ISP_BD add
typedef struct _ISP_BD
{
    u8     ISP_running;//  远程下载进行中
    u16    ISP_runTimer;// 进行中 计时器
    u16   Total_PacketNum;//  总包数
    u16   CurrentPacket_Num;//  当前报数
    u16   PacketSizeGet;
    u32   PacketRX_wr;//  接收数据累加长度
    u8     Update_Type;  // 终端数据类型
    u8  ProductID[5];// 制造商编号
    u8  Version_len; // 版本号长度
    u8  VersionStr[20];//  版本号
    u32  Content_len;// 升级包长度
    u8   ContentData[1024]; // 数据包内容
    u8   ISP_resualt;  //升级结果
    u8   ISP_status_Read;  // 上电读取 DF 状态
} ISP_BD;


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
    u8      LORA_TYPE;     //  节点类型           0:  中继   1: 雷达监测点   2:  道口接收  3 :手持终端

} SYS_CONF;


extern  u8           Login_Menu_Flag;	   //	登陆界面 标志位
extern 	u8			 Limit_max_SateFlag;	  //   速度最大门限限制指令


//-----  ISP    远程下载相关 -------
extern ISP_BD  BD_ISP; //  BD   升级包


extern  u32 PicFileSize; // 图片文件大小
extern  u8  PictureName[40];
extern  u32 TimeTriggerPhoto_counter;
extern  u32 Timer_stop_taking_timer;  //   终止定时拍照 计时器 



//------  voice -----



//------  video  --------



//-------  行车记录仪相关extern BD_EXTEND     BD_EXT;     //  北斗扩展协议
extern  u8         Vehicle_sensor; // 车辆传感器状态   0.2s  查询一次
extern  u8		   Vehicle_sensor_Bak;  // BAK 

extern 	u8		   save_sensorCounter, sensor_writeOverFlag;
extern u32   Delta_1s_Plus;


extern u8 TextInforCounter;//文本信息条数
extern DOORCamera   DoorOpen;    //  开关车门拍照
extern MEDIA_INDEX  MediaIndex;  // 多媒体信息
extern INOUT        InOut_Object;  // 进出围栏状态
extern DATATRANS    DataTrans;     // 数据信息透传
extern MULTIMEDIA   MediaObj;      // 多媒体信息
extern VOICE_RECODE VoiceRec_Obj;   //  录音功能
extern CAMERA       Camera_Obj;     //  中心拍照相关
extern RECODER      Recode_Obj;     // 行车记录仪
//-------文本信息-------
extern MSG_TEXT       TEXT_Obj;
extern MSG_TEXT       TEXT_Obj_8[8], TEXT_Obj_8bak[8];

extern TEXT_INFO      TextInfo;    // 文本信息下发

extern SPD_EXP    speed_Exd;      // 超速报警
extern DevRegst   DEV_regist;  // 注册
extern DevLOGIN   DEV_Login;   //  鉴权


extern NANDSVFlag   NandsaveFlg;
extern A_AckFlag    Adata_ACKflag;
extern TCP_ACKFlag  SD_ACKflag;



extern u32           SubCMD_8103H;            //  02 H命令 设置记录仪安装参数回复 子命令
extern u32         SubCMD_FF01H;            //  FF02 北斗信息扩展
extern u32         SubCMD_FF03H;     //  FF03  设置扩展终端参数设置1

extern u8          SubCMD_10H;            //  10H   设置记录仪定位告警参数
extern u8	       OutGPS_Flag; // 0  默认  1  接外部有源天线
extern u8	       Spd_senor_Null;  // 手动传感器速度为0
extern u32         Centre_DoubtRead;     //  中心读取事故疑点数据的读字段
extern u32         Centre_DoubtTotal;    //  中心读取事故疑点的总字段


extern u8 		   FCS_GPS_UDP;						//UDP 数据异或和
extern u8          FCS_RX_UDP;                       // UDP 数据接收校验
extern u8          Centre_IP_modify;            //  中修改IP了
extern u8          IP_change_counter;           //   中心修改IP 计数器
extern u8		   Down_Elec_Flag;				 //   断油断电使能标志位


extern ALIGN(RT_ALIGN_SIZE) u8          GPRS_info[1400];
extern u16         GPRS_infoWr_Tx;


//------ phone
extern u8       CallState; // 通话状态




//--------  GPS prototcol------------------------------------------------------------------------------------------------
extern u8   UDP_dataPacket_flag;			   /*V	   0X03 	 ;		   A	  0X02*/
extern u8   DispContent;   // 发送时是否显示数据内容



extern u8	GPS_getfirst; 		 //  首次有经纬度
extern u8	HDOP_value;		 //  Hdop 数值
extern u8   Satelite_num;   // 卫星颗数

extern u8   CurrentTime[3];
extern u8   BakTime[3];
extern u8   Sdgps_Time[3];  // GPS 发送 时间记录


extern  ALIGN(RT_ALIGN_SIZE) u8  UDP_HEX_Rx[1024];    // EM310 接收内容hex

extern u16 UDP_hexRx_len;    // hex 内容 长度
extern u16 UDP_DecodeHex_Len;// UDP接收后808 解码后的数据长度



extern GPS_RMC  GPRMC;                   // GPMC格式
//---------- 808 协议 -----------------------------------------------------------------------------------------------
extern 	u16 	        GPS_Hight;			   //	808协议-> 高程	 m
extern  u16		        Speed_gps;    // 通过GPS计算出来的速度 km/h
extern   u16            Speed_jiade; //  假的速度 1: enable 0: disable

extern 	u16	            Spd_Using;								  //  当前使用的速度 0.1 km/h
extern  u32		        Sps_larger_5_counter;    //	GPS  using	大于   5km/h  计数器
extern  u16			    GPS_direction;							  //  GPS方向		 单位2度

//---------- 用GPS校准特征系数相关 ----------------------------
extern u8		Speed_area; // 校验K值范围
extern u16		Speed_cacu; // 通过K值计算出来的速度
extern u16 	    Speed_cacu_BAK;  //  传感器  备份
extern u8       Speed_cacu_Trigger_Flag;

extern u16 	    Spd_adjust_counter; // 确保匀速状态计数器
extern u16      Former_DeltaPlus[K_adjust_Duration]; // 前几秒的脉冲数
extern u8       Former_gpsSpd[K_adjust_Duration];// 前几秒的速度
//------- 车辆负载状态 ---------------
extern u8      CarLoadState_Flag;//选中车辆状态的标志   1:空车   2:半空   3:重车
//------- 多媒体信息类型---------------
extern u8  Multimedia_Flag;//需要上传的多媒体信息类型	1:视频	 2:音频   3:图像
extern u8  SpxBuf[SpxBuf_Size];
extern u16 Spx_Wr, Spx_Rd;
extern u8	Duomeiti_sdFlag;

//------- 录音开始或者结束---------------
extern u8  Recor_Flag; //  1:录音开始   2:录音结束


// ---- 拐点 -----
extern u16  Inflexion_Current;
extern u16  Inflexion_Bak;
extern u16  Inflexion_chgcnter; //变化计数器
extern u16  InflexLarge_or_Small;	    // 判断curent 和 Bak 大小	0 equql  1 large  2 small
extern u16  InflexDelta_Accumulate;	//	差值累计


// ----休眠状态  ------------
extern u8   SleepState;  //   0  不休眠ACC on        1  休眠Acc Off
extern u8	SleepConfigFlag; //  休眠时发送鉴权标志位

//---- 固定文件大小 ---
extern u32 mp3_fsize;
extern u32 wmv_fsize;
extern u8  mp3_sendstate;
extern u8  wmv_sendstate;



//---------- 河北天地通协议 ------------------------------------------------------------------------------------------
extern  u8		SIM_code[6];							   // 要发送的IMSI	号码
extern  u8		IMSI_CODE[15];							//SIM 卡的IMSI 号码
extern  u8		Warn_Status[4]; // 报警状态信息
extern 	u8	    Warn_MaskWord[4];	//	报警屏蔽字
extern  u8      Text_MaskWord[4];
extern  u8      Key_MaskWord[4]; //   关键字屏蔽字


extern  u8		Car_Status[4];  // 车辆状态信息
extern T_GPS_Info_GPRS 	 Gps_Gprs;
extern T_GPS_Info_GPRS	 Temp_Gps_Gprs;
extern u8	    A_time[6]; // 定位时刻的时间
// 定位时刻的时间
extern u8       ReadPhotoPageTotal;
extern u8       SendPHPacketFlag; ////收到中心启动接收下一个block时置位

//-------- 紧急报警 --------
extern u8		warn_flag;
extern u8		f_Exigent_warning;//0;     //脚动 紧急报警装置 (INT0 PD0)
extern u8		Send_warn_times;     //   设备向中心上报报警次数，最大3 次
extern u32  	    fTimer3s_warncount;


extern POSIT Posit[60];                //  每分钟位置信息存储
extern u8    PosSaveFlag;             //  存储Pos 状态位

extern u8   Vehicle_RunStatus;    //  bit 0: ACC 开 关             1 开  0关
//  bit 1: 通过速度传感器感知    1 表示行驶  0 表示停止
//  bit 2: 通过gps速度感知       1 表示行驶  0 表示停止


extern u8    SpxSrcName[13];
extern u32   SrcFileSize, DestFilesize, SrcFile_read;
extern u8	 SleepCounter;

extern u16   DebugSpd;	//调试用GPS速度
extern u8	 MMedia2_Flag;
extern u8		 ACK_timer;				   //---------	ACK timer 定时器---------------------
extern u8     Send_Rdy4ok;

extern u8   Flag_0200_send; // 发送0200  flag
extern u16  Timer_0200_send; // 0200  判断应答


//-------------    不同北斗模块设置  ----
//#ifdef HC_595_CONTROL
//---------74CH595  Q5   control Power----
extern u8   Print_power_Q5_enable;
extern u8   Q7_enable;
//#endif






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
/*
     -----------------------------
     2.2 输入管脚状态监测
     -----------------------------
*/

/*
     -----------------------------
    2.3  控制输出
     -----------------------------
*/
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


