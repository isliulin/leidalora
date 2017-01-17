/*
     Protocol_808.C
*/

#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>
#include "math.h"
#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "math.h"
#include "stdarg.h"
#include "string.h"
#include "SMS.h"




#define    ROUTE_DIS_Default            0x3F000000


u8 chushilicheng[4];
u8 Setting08[80] = "预留 	    车门  	   雾灯  	   近光灯	   远光灯	   右转灯	   左转灯	   刹车 	    ";




//------ Photo -----
u32 PicFileSize = 0; // 图片文件大小
u8  PictureName[40];



//------  voice -----



//------  video  --------


/*
             杂
*/
//------ phone
u8       CallState = CallState_Idle; // 通话状态

//   ASCII  to   GB    ---- start
//0-9        10
u8  arr_A3B0[20] = {0xA3, 0xB0, 0xA3, 0xB1, 0xA3, 0xB2, 0xA3, 0xB3, 0xA3, 0xB4, 0xA3, 0xB5, 0xA3, 0xB6, 0xA3, 0xB7, 0xA3, 0xB8, 0xA3, 0xB9};

//@ A-O      16
u8  arr_A3C0[32] = {0xA3, 0xC0, 0xA3, 0xC1, 0xA3, 0xC2, 0xA3, 0xC3, 0xA3, 0xC4, 0xA3, 0xC5, 0xA3, 0xC6, 0xA3, 0xC7, 0xA3, 0xC8, 0xA3, 0xC9, 0xA3, 0xCA, 0xA3, 0xCB, 0xA3, 0xCC, 0xA3, 0xCD, 0xA3, 0xCE, 0xA3, 0xCF};

//P-Z         11个
u8  arr_A3D0[22] = {0xA3, 0xD0, 0xA3, 0xD1, 0xA3, 0xD2, 0xA3, 0xD3, 0xA3, 0xD4, 0xA3, 0xD5, 0xA3, 0xD6, 0xA3, 0xD7, 0xA3, 0xD8, 0xA3, 0xD9, 0xA3, 0xDA};

//.  a-0       16
u8  arr_A3E0[32] = {0xA3, 0xE0, 0xA3, 0xE1, 0xA3, 0xE2, 0xA3, 0xE3, 0xA3, 0xE4, 0xA3, 0xE5, 0xA3, 0xE6, 0xA3, 0xE7, 0xA3, 0xE8, 0xA3, 0xE9, 0xA3, 0xEA, 0xA3, 0xEB, 0xA3, 0xEC, 0xA3, 0xED, 0xA3, 0xEE, 0xA3, 0xEF};

//p-z          11
u8  arr_A3F0[22] = {0xA3, 0xF0, 0xA3, 0xF1, 0xA3, 0xF2, 0xA3, 0xF3, 0xA3, 0xF4, 0xA3, 0xF5, 0xA3, 0xF6, 0xA3, 0xF7, 0xA3, 0xF8, 0xA3, 0xF9, 0xA3, 0xFA};
//-------  ASCII to GB ------



//----------- 行车记录仪相关  -----------------
u8          Vehicle_sensor = 0; // 车辆传感器状态   0.2s  查询一次
u8          Vehicle_sensor_Bak=0;  // BAK
/*
D7  刹车
D6  左转灯
D5  右转灯
D4  喇叭
D3  远光灯
D2  雨刷
D1  预留
D0  预留
*/

u8          save_sensorCounter = 0, sensor_writeOverFlag = 0;;


u8       DispContent = 0; // 发送时是否显示数据内容
/*
            1 <->  正常显示
            2 <->  显示发送信息的
            3 <->  显示 任务的运行情况
            0<-> 不显示调试输出，只显示协议数据
     */

u8         TextInforCounter = 0; //文本信息条数

u8 		   FCS_GPS_UDP = 0;						//UDP 数据异或和
u8         FCS_RX_UDP = 0;                     // UDP 数据接收校验
u8         FCS_error_counter = 0;              // 校验错误计数器

u8          Centre_IP_modify = 0;             //  中修改IP了
u8          IP_change_counter = 0;           //   中心修改IP 计数器
u8          Down_Elec_Flag = 0;              //   断油断电使能标志位



//---------74CH595  Q5   control Power----
u8   Print_power_Q5_enable = 0;
u8   Q7_enable = 0;







//------------ 超速报警---------------------
SPD_EXP speed_Exd;


//--------  GPS prototcol----------------------------------------------------------------------------------
static u32 	fomer_time_seconds, tmp_time_secnonds, delta_time_seconds;
u8	        UDP_dataPacket_flag = 'V';			  /*V	   0X03 	 ;		   A	  0X02*/
u8          Year_illigel = 0; //  年份不合法
u8	        GPS_getfirst = 0; 		 //  首次有经纬度
u8          HDOP_value = 99;       //  Hdop 数值
u8          Satelite_num = 0; // 卫星颗数
u8 CurrentTime[3];
u8 BakTime[3];
u8 Sdgps_Time[3];  // GPS 发送 时间记录   BCD 方式

//u16  Spd_add_debug=0;

//static u8      UDP_AsciiTx[1800];
ALIGN(RT_ALIGN_SIZE)
u8      GPRS_info[1400];
u16     GPRS_infoWr_Tx = 0;

ALIGN(RT_ALIGN_SIZE)
u8  UDP_HEX_Rx[1024];    // EM310 接收内容hex

u16 UDP_hexRx_len = 0;  // hex 内容 长度
u16 UDP_DecodeHex_Len = 0; // UDP接收后808 解码后的数据长度


GPS_RMC GPRMC; // GPMC格式

/*                         pGpsRmc->status,\
						pGpsRmc->latitude_value,\
						pGpsRmc->latitude,\
						pGpsRmc->longtitude_value,\
						pGpsRmc->longtitude,\
						pGpsRmc->speed,\
						pGpsRmc->azimuth_angle);
						*/



//----------808 协议 -------------------------------------------------------------------------------------
u16	   GPS_Hight = 0;             //   808协议-> 高程   m
u16     GPS_direction = 0;         //   808协议-> 方向   度
u16     Centre_FloatID = 0; //  中心消息流水号
u16     Centre_CmdID = 0; //  中心命令ID

u8      Original_info[1400]; // 没有转义处理前的原始信息
u16     Original_info_Wr = 0; // 原始信息写地址
//---------- 用GPS校准特征系数相关 ----------------------------
u8      Speed_area = 60; // 校验K值范围
u16	    Spd_Using = 0;			 //   808协议-> 速度   0.1km/h      当前使用的速度，判断超速疲劳的依据
u32     Sps_larger_5_counter = 0;  //   GPS  using  大于   5km/h  计数器
u16     Speed_gps = 0; // 通过GPS计算出来的速度 0.1km/h
u16     Speed_jiade = 0; //  假的速度   1: enable 0: disable
u8      Speed_Rec = 0; // 速度传感器 校验K用的存储器
u16     Speed_cacu = 0; // 通过K值计算出来的速度    通过传感器获取的速度
u16     Speed_cacu_BAK = 0; //  传感器  备份
u8      Speed_cacu_Trigger_Flag = 0;
u16     Spd_adjust_counter = 0; // 确保匀速状态计数器
u16     Spd_Deltacheck_counter = 0; // 传感器速度和脉冲速度相差较大判断
u16     Former_DeltaPlus[K_adjust_Duration]; // 前几秒的脉冲数
u8      Former_gpsSpd[K_adjust_Duration];// 前几秒的速度
u8      Illeagle_Data_kickOUT = 0; //  剔除非法数据状态

//-----  车台注册定时器  ----------
DevRegst   DEV_regist;  // 注册
DevLOGIN   DEV_Login;   //  鉴权




//------- 文本信息下发 -------
TEXT_INFO      TextInfo;    // 文本信息下发
//-------文本信息-------
MSG_TEXT       TEXT_Obj;
MSG_TEXT       TEXT_Obj_8[8], TEXT_Obj_8bak[8];

//------ 提问  --------
CENTRE_ASK     ASK_Centre;  // 中心提问

//-----  车辆控制 ------
VEHICLE_CONTROL Vech_Control; //  车辆控制
//-------    行车记录仪  -----
RECODER      Recode_Obj;     // 行车记录仪
//-------  拍照  ----
CAMERA        Camera_Obj;     //  中心拍照相关
//-----   录音  ----
VOICE_RECODE  VoiceRec_Obj;   //  录音功能
//------ 多媒体  --------
MULTIMEDIA    MediaObj;       // 多媒体信息
//-------  数据信息透传  -------
DATATRANS     DataTrans;      // 数据信息透传
//-------  进出围栏状态 --------
INOUT        InOut_Object;    // 进出围栏状态
//-------- 多媒体检索  ------------
MEDIA_INDEX  MediaIndex;  // 多媒体信息
//------- 车辆负载状态 ---------------
u8  CarLoadState_Flag = 1; //选中车辆状态的标志   1:空车   2:半空   3:重车

//------- 多媒体信息类型---------------
u8  Multimedia_Flag = 1; //需要上传的多媒体信息类型   1:视频   2:音频   3:图像
u8  SpxBuf[SpxBuf_Size];
u16 Spx_Wr = 0, Spx_Rd = 0;
u8  Duomeiti_sdFlag = 0;

//------- 录音开始或者结束---------------
u8  Recor_Flag = 1; //  1:录音开始   2:录音结束

//----------808协议 -------------------------------------------------------------------------------------
u8		SIM_code[6];							   // 要发送的IMSI	号码
u8		IMSI_CODE[15] = "000000000000000";							//SIM 卡的IMSI 号码
u8		Warn_Status[4]		=
{
    0x00, 0x00, 0x00, 0x00
}; //  报警标志位状态信息
u8  Warn_MaskWord[4]		=
{
    0x00, 0x00, 0x00, 0x00
};   //  报警屏蔽字
u8  Text_MaskWord[4] =
{
    0x00, 0x00, 0x00, 0x00
};	 //  文本屏蔽字
u8  Key_MaskWord[4] =
{
    0x00, 0x00, 0x00, 0x00
};	 //   关键字屏蔽字



u8		Car_Status[4]		=
{
    0x00, 0x0c, 0x00, 0x00
}; //  车辆状态信息
T_GPS_Info_GPRS 	Gps_Gprs, Bak_GPS_gprs;
T_GPS_Info_GPRS	Temp_Gps_Gprs;
u8   A_time[6]; // 定位时刻的时间

u8      ReadPhotoPageTotal = 0;
u8      SendPHPacketFlag = 0; ////收到中心启动接收下一个block时置位


//-------- 紧急报警 --------
u8		warn_flag = 0;
u8		f_Exigent_warning = 0; //0;     //脚动 紧急报警装置 (INT0 PD0)
u8		Send_warn_times = 0;    //   设备向中心上报报警次数，最大3 次
u32  	fTimer3s_warncount = 0;

// ------  车辆信息单独了 ---------------
VechINFO     Vechicle_Info;     //  车辆信息
VechINFO     Vechicle_Info_BAK;  //  车辆信息 BAK
VechINFO     Vechicle_info_BAK2; //  车辆信息BAK2
u8           Login_Menu_Flag = 1;     //   登陆界面 标志位
u8           Limit_max_SateFlag = 0;  //   速度最大门限限制指令


//------  车门开关拍照 -------
DOORCamera   DoorOpen;    //  开关车门拍照

//------- 北斗扩展协议  ------------
BD_EXTEND     BD_EXT;     //  北斗扩展协议
DETACH_PKG   Detach_PKG; // 分包重传相关
SET_QRY         Setting_Qry; //  终端参数查询
u32     CMD_U32ID = 0;
PRODUCT_ATTRIBUTE   ProductAttribute;// 终端属性
HUMAN_CONFIRM_WARN   HumanConfirmWarn;// 人工确认报警

//-----  ISP    远程下载相关 -------
ISP_BD  BD_ISP; //  BD   升级包



// ---- 拐点 -----
u16  Inflexion_Current = 0;
u16  Inflexion_Bak = 0;
u16  Inflexion_chgcnter = 0; //变化计数器
u16  InflexLarge_or_Small = 0;   // 判断curent 和 Bak 大小    0 equql  1 large  2 small
u16  InflexDelta_Accumulate = 0; //  差值累计

// ----休眠状态  ------------
u8  SleepState = 0; //   0  不休眠ACC on            1  休眠Acc Off
u8  SleepConfigFlag = 0; //  休眠时发送鉴权标志位

//---- 固定文件大小 ---
u32 mp3_fsize = 5616;
u8  mp3_sendstate = 0;
u32 wmv_fsize = 25964;
u8  wmv_sendstate = 0;

//-------------------   公共 ---------------------------------------
static u8 GPSsaveBuf[128];     // 存储GPS buffer
static u8	ISP_buffer[1024];
static u16 GPSsaveBuf_Wr = 0;


POSIT Posit[60];           // 每分钟位置信息存储
u8    PosSaveFlag = 0;    // 存储Pos 状态位

NANDSVFlag   NandsaveFlg;
A_AckFlag    Adata_ACKflag;  // 无线GPRS协议 接收相关 RS232 协议返回状态寄存器
TCP_ACKFlag  SD_ACKflag;     // 无线GPRS协议返回状态标志
u32  SubCMD_8103H = 0;          //  02 H命令 设置记录仪安装参数回复 子命令
u32  SubCMD_FF01H = 0;          //  FF02 北斗信息扩展
u32  SubCMD_FF03H = 0;   //  FF03  设置扩展终端参数设置1
u8   Fail_Flag = 0;


u8  SubCMD_10H = 0;          //  10H   设置记录仪定位告警参数
u8  OutGPS_Flag = 0;   //  0  默认  1  接外部有源天线
u8   Spd_senor_Null = 0; // 手动传感器速度为0
u32  Centre_DoubtRead = 0;   //  中心读取事故疑点数据的读字段
u32  Centre_DoubtTotal = 0;  //  中心读取事故疑点的总字段
u8   Vehicle_RunStatus = 0;  //  bit 0: ACC 开 关             1 开  0关
//  bit 1: 通过速度传感器感知    1 表示行驶  0 表示停止
//  bit 2: 通过gps速度感知       1 表示行驶  0 表示停止



u32   SrcFileSize = 0, DestFilesize = 0, SrcFile_read = 0;
u8    SleepCounter = 0;

u16   DebugSpd = 0; //调试用GPS速度
u8    MMedia2_Flag = 0; // 上传固有音频 和实时视频  的标志位    0 传固有 1 传实时


u8	 reg_128[128];  // 0704 寄存器

unsigned short int FileTCB_CRC16 = 0;
unsigned short int Last_crc = 0, crc_fcs = 0;



//---------  中心应答  -----------
u8		 ACK_timer = 0;				 //---------	ACK timer 定时器---------------------
u8           Send_Rdy4ok = 0;
unsigned char	Rstart_time = 0;

u8   Flag_0200_send=0; // 发送0200  flag
u16  Timer_0200_send=0; // 0200  判断应答


//---------------  速度脉冲相关-------------- 
u32  Delta_1s_Plus = 0;
u16  Sec_counter = 0;
u32  TimeTriggerPhoto_counter = 0; // 定时触发拍照计时器
u32  Timer_stop_taking_timer=0;  //   终止定时拍照 计时器 
//void Video_send_end(void);
unsigned short int  File_CRC_Get(void);

//  A.  Total

void delay_us(u16 j)
{
    u8 i;
    while(j--)
    {
        i = 3;
        while(i--);
    }
}

void delay_ms(u16 j )
{
    while(j--)
    {
        DF_delay_us(2000); // 1000
    }
}

u8  Do_SendGPSReport_GPRS(void)
{
  #if  0       // Lora  Used Demo
   if(Flag_enable)
   {
     Lora_Gprs_Send();
    }
   #endif 
   return true;
}
void strtrim(u8 *s, u8 c)
{
    u8		 *p1, *p2;
    u16  i, j;

    if (s == 0) return;

    // delete the trailing characters
    if (*s == 0) return;
    j = strlen((char const *)s);
    p1 = s + j;
    for (i = 0; i < j; i++)
    {
        p1--;
        if (*p1 != c) break;
    }
    if (i < j) p1++;
    *p1 = 0;	// null terminate the undesired trailing characters

    // delete the leading characters
    p1 = s;
    if (*p1 == 0) return;
    for (i = 0; *p1++ == c; i++);
    if (i > 0)
    {
        p2 = s;
        p1--;
        for (; *p1 != 0;) *p2++ = *p1++;
        *p2 = 0;
    }
}

int str2ip(char *buf, u8 *ip)
{
    // convert an ip:port string into a binary values
    int	i;
    u16	_ip[4];


    memset(_ip, 0, sizeof(_ip));

    strtrim((u8 *)buf, ' ');

    i = sscanf(buf, "%u.%u.%u.%u", (u32 *)&_ip[0], (u32 *)&_ip[1], (u32 *)&_ip[2], (u32 *)&_ip[3]);

    *(u8 *)(ip + 0) = (u8)_ip[0];
    *(u8 *)(ip + 1) = (u8)_ip[1];
    *(u8 *)(ip + 2) = (u8)_ip[2];
    *(u8 *)(ip + 3) = (u8)_ip[3];

    return i;
}



int IP_Str(char *buf, u32 IP)
{

    if (!buf) return 0;



    return 1;
}

u16 AsciiToGb(u8 *dec, u8 InstrLen, u8 *scr)
{
    u16 i = 0, j = 0, m = 0;
    u16 Info_len = 0;


    for(i = 0, j = 0; i < InstrLen; i++, j++)
    {
        m = scr[i];
        if((m >= 0x30) && (m <= 0x39))
        {
            memcpy(&dec[j], &arr_A3B0[(m - '0') * 2], 2);
            j++;
        }
        else if((m >= 0x41) && (m <= 0x4f))
        {
            memcpy(&dec[j], &arr_A3C0[(m - 0x41 + 1) * 2], 2);
            j++;
        }
        else if((m >= 0x50) && (m <= 0x5a))
        {
            memcpy(&dec[j], &arr_A3D0[(m - 0x50) * 2], 2) ;
            j++;
        }
        else if((m >= 0x61) && (m <= 0x6f))
        {
            memcpy(&dec[j], &arr_A3E0[(m - 0x61 + 1) * 2], 2) ;
            j++;
        }
        else if((m >= 0x70) && (m <= 0x7a))
        {
            memcpy(&dec[j], &arr_A3F0[(m - 0x70) * 2], 2)  ;
            j++;
        }
        else
        {
            dec[j] = m;
        }
    }
    Info_len = j;
    return Info_len;
}



//==================================================================================================
// 第三部分 :   以下是GPRS无线传输相关协议
//==================================================================================================
void  Save_GPS(void)
{

}
//-------------------- ISP Check  ---------------------------------------------
void  ISP_file_Check(void)
{
    u8  FileHead[100];
    u8  ISP_judge_resualt = 0;
    u32  HardVersion = 0;

    memset(ISP_buffer, 0, sizeof(ISP_buffer));
    SST25V_BufferRead(ISP_buffer, ISP_StartArea, 256);
    //---判断文件更新标志---------------------
    if(ISP_buffer[32] != ISP_BYTE_CrcPass) //  计算校验通过后  更新标志改成0xE1     以前是0xF1
    {
        rt_kprintf("\r\n 厂商型号正确\r\n");
        return;
    }

    /*
       序号   字节数	名称			  备注
      1 		  1    更新标志 	 1 表示需要更新   0 表示不需要更新
      2-5			  4   设备类型				 0x0000 0001  ST712   TWA1
    									0x0000 0002   STM32  103  新A1
    									0x0000 0003   STM32  101  简易型
    									0x0000 0004   STM32  A3  sst25
    									0x0000 0005   STM32  行车记录仪
      6-9		 4	   软件版本 	 每个设备类型从  0x0000 00001 开始根据版本依次递增
      10-29 	  20	日期		' mm-dd-yyyy HH:MM:SS'
      30-31 	  2    总页数		   不包括信息页
      32-35 	  4    程序入口地址
      36-200	   165	  预留
      201-		  n    文件名

    */
    //------------   Type check  ---------------------
    memset(FileHead, 0, sizeof(FileHead));
    memcpy(FileHead, ISP_buffer, 32);
    rt_kprintf( "\r\n FileHead:%s\r\n", FileHead );


    //------    文件格式判断
    if(strncmp(ISP_buffer + 32 + 13, "70420TW705", 10) == 0) //判断厂商和型号
    {
        ISP_judge_resualt++;// step 1
        rt_kprintf("\r\n 厂商型号正确\r\n");

        // hardware
        HardVersion = (ISP_buffer[32 + 38] << 24) + (ISP_buffer[32 + 39] << 16) + (ISP_buffer[32 + 40] << 8) + ISP_buffer[32 + 41];
        HardWareVerion = HardWareGet();
        if(HardWareVerion == HardVersion)	// 要兼容以前的老板子 全1
        {
            ISP_judge_resualt++;// step 2
            rt_kprintf("\r\n 硬件版本:%d\r\n", HardVersion);
        }
        else
            rt_kprintf("\r\n 硬件版本不匹配!\r\n");
        //firmware
        if(strncmp((const char *)ISP_buffer + 32 + 42, "NXBXGGHYPT", 10) == 0)
        {
            ISP_judge_resualt++;// step 3
            rt_kprintf("\r\n  固件版本:NXBXGGHYPT\r\n");
        }
        // operater
        if(strncmp((const char *)ISP_buffer + 32 + 62, "NXBX", 4) == 0)
        {
            ISP_judge_resualt++;// step 4
            rt_kprintf("\r\n  固件版本:NXBX\r\n");
        }

    }

    //ISP_judge_resualt=4;
    if(ISP_judge_resualt == 4)
    {
        //------- enable  flag -------------
        SST25V_BufferRead( FileHead, 0x001000, 100 );
        FileHead[32] = ISP_BYTE_Rdy2Update;    //-----  文件更新标志  使能启动时更新
        SST25V_BufferWrite( FileHead, 0x001000, 100);

        {
            Systerm_Reset_counter = (Max_SystemCounter - 5);	 // 准备重启更新最新程序
            ISP_resetFlag = 1; //准备重启
            rt_kprintf( "\r\n 准备重启更新程序!\r\n" );
        }

        // rt_kprintf( "\r\n 升级完成了，但不更新等待判断 校验测试 \r\n" );
    }
    else
    {
        //------- enable  flag -------------
        SST25V_BufferRead( FileHead, 0x001000, 100 );
        FileHead[32] = ISP_BYTE_TypeNotmatch;    //-----   文件校验通过，但 类型不匹配
        SST25V_BufferWrite( FileHead, 0x001000, 100);
        BD_ISP.ISP_running = 0; // recover normal
        rt_kprintf( "\r\n 相关参数不匹配不与更新!\r\n" );
    }

}
// FINSH_FUNCTION_EXPORT(ISP_file_Check, ISP_file_Check);

u16  Instr_2_GBK(u8 *SrcINstr, u16 Inlen, u8 *DstOutstr )
{
    u16 i = 0, j = 0;


    //对非GBK编码处理------------------------------------
    for(i = 0, j = 0; i < Inlen; i++)
    {
        if((SrcINstr[i] >= 0xA1) && (SrcINstr[i + 1] >= 0xA0))
        {
            DstOutstr[j] = SrcINstr[i];
            DstOutstr[j + 1] = SrcINstr[i + 1];
            j += 2;
            i++;
        }
        else
        {
            DstOutstr[j] = ' ';
            DstOutstr[j + 1] = SrcINstr[i];
            j += 2;
        }
    }
    return   j;
}


//-----------------------------------------------------------
void TCP_RX_Process( u8  LinkNum)  //  ---- 808  标准协议
{
    // UDP_HEX_Rx,UDP_hexRx_len   is  rx info 
    //-----------------  memset  -------------------------------------
    //memset(UDP_HEX_Rx, 0, sizeof(UDP_HEX_Rx));
    //UDP_hexRx_len= 0;
    return;

}


void Time2BCD(u8 *dest)
{
#if 0
    if(UDP_dataPacket_flag == 0x02)
    {

        dest[0] = ((Temp_Gps_Gprs.Date[0] / 10) << 4) + (Temp_Gps_Gprs.Date[0] % 10);
        dest[1] = ((Temp_Gps_Gprs.Date[1] / 10) << 4) + (Temp_Gps_Gprs.Date[1] % 10); //Temp_Gps_Gprs.Date[1];
        dest[2] = ((Temp_Gps_Gprs.Date[2] / 10) << 4) + (Temp_Gps_Gprs.Date[2] % 10); //Temp_Gps_Gprs.Date[2];

        dest[3] = ((Temp_Gps_Gprs.Time[0] / 10) << 4) + (Temp_Gps_Gprs.Time[0] % 10); //Temp_Gps_Gprs.Time[0];
        dest[4] = ((Temp_Gps_Gprs.Time[1] / 10) << 4) + (Temp_Gps_Gprs.Time[1] % 10); //Temp_Gps_Gprs.Time[1];
        dest[5] = ((Temp_Gps_Gprs.Time[2] / 10) << 4) + (Temp_Gps_Gprs.Time[2] % 10); //Temp_Gps_Gprs.Time[2];

    }
    else
#endif
    {
        dest[0] = ((time_now.year / 10) << 4) + (time_now.year % 10);
        dest[1] = ((time_now.month / 10) << 4) + (time_now.month % 10);
        dest[2] = ((time_now.day / 10) << 4) + (time_now.day % 10);
        dest[3] = ((time_now.hour / 10) << 4) + (time_now.hour % 10);
        dest[4] = ((time_now.min / 10) << 4) + (time_now.min % 10);
        dest[5] = ((time_now.sec / 10) << 4) + (time_now.sec % 10);
    }

}

unsigned short int File_CRC_Get(void)
{

    u8   buffer_temp[514];
    unsigned short int i = 0;
    u16  packet_num = 0, leftvalue = 0; // 512    per packet
    u32  File_size = 0;

    DF_TAKE;
    memset(buffer_temp, 0, sizeof(buffer_temp));

    //  获取  文件头信息
    SST25V_BufferRead( buffer_temp, ISP_StartArea,  256 );
    File_size = (buffer_temp[114] << 24) + (buffer_temp[115] << 16) + (buffer_temp[116] << 8) + buffer_temp[117];

    leftvalue = File_size % 512;
    rt_kprintf("\r\n 文件大小: %d Bytes  leftvalue=%d \r\n", File_size, leftvalue);
    FileTCB_CRC16 = (buffer_temp[134] << 8) + buffer_temp[135];
    rt_kprintf("\r\n Read CRC16: 0x%X Bytes\r\n", FileTCB_CRC16);

    // OutPrint_HEX("1stpacket",buffer_temp,256);

    if(leftvalue)   // 除以 512
        packet_num = (File_size >> 9) + 1;
    else
        packet_num = (File_size >> 9);


    for(i = 0; i < packet_num; i++)
    {
        if(i == 0) //第一包
        {
            Last_crc = 0; // clear first
            crc_fcs = 0;
            SST25V_BufferRead(buffer_temp, ISP_StartArea + 256, 512); // 0x001000+256=0x001100   ISP_StartArea+256
            delay_ms(50);
            WatchDog_Feed();
            Last_crc = CRC16_1(buffer_temp, 512, 0xffff);
            //rt_kprintf("\r\n                  i=%d,Last_crc=0x%X",i+1,Last_crc);

            //rt_kprintf("\r\n //----------   %d     packet    len=%d  ",i+1,512);
            //OutPrint_HEX("1stpacket",buffer_temp,512);
        }
        else if(i == (packet_num - 1)) //最后一包
        {
            SST25V_BufferRead(buffer_temp, ISP_StartArea + 256 + i * 512, leftvalue);
            delay_ms(50);
            WatchDog_Feed();
            // rt_kprintf("\r\n //----------   %d     packet    len=%d  ",i+1,leftvalue);
            // OutPrint_HEX("endstpacket",buffer_temp,leftvalue);
            crc_fcs = CRC16_1(buffer_temp, leftvalue, Last_crc);
            rt_kprintf("\r\n                  i=%d,Last_crc=0x%X ReadCrc=0x%X ", i + 1, crc_fcs, FileTCB_CRC16);
        }
        else
        {
            // 中间的包
            SST25V_BufferRead(buffer_temp, ISP_StartArea + 256 + i * 512, 512);
            delay_ms(50);
            WatchDog_Feed();
            // rt_kprintf("\r\n //----------   %d	 packet    len=%d  ",i+1,512);
            // OutPrint_HEX("midstpacket",buffer_temp,512);
            Last_crc = CRC16_1(buffer_temp, 512, Last_crc);
            // rt_kprintf("\r\n                 i=%d,Last_crc=0x%X",i+1,Last_crc);
        }
    }

    DF_RELEASE;
    rt_kprintf("\r\n  校验结果 0x%X \r\n", crc_fcs);

    if(FileTCB_CRC16 == crc_fcs)
    {
        SST25V_BufferRead( buffer_temp, 0x001000, 100 );
        buffer_temp[32] = ISP_BYTE_CrcPass;    //-----   文件校验通过
        SST25V_BufferWrite( buffer_temp, 0x001000, 100);
        rt_kprintf("\r\n  校验正确! \r\n", crc_fcs);
        return true;
    }
    else
    {
        rt_kprintf("\r\n  校验失败! \r\n", crc_fcs);
        return false;
    }
}
//FINSH_FUNCTION_EXPORT(File_CRC_Get, File_CRC_Get);


/*
    打印输出 HEX 信息，Descrip : 描述信息 ，instr :打印信息， inlen: 打印长度
*/
void OutPrint_HEX(u8 *Descrip, u8 *instr, u16 inlen )
{
    u32  i = 0;
    rt_kprintf("\r\n %s:", Descrip);
    for( i = 0; i < inlen; i++)
        rt_kprintf("%02X ", instr[i]);
    rt_kprintf("\r\n");
}

void  redial(void)
{
    DataLink_EndFlag = 1; //AT_End();
    // rt_kprintf("\r\n Redial\r\n");
}
FINSH_FUNCTION_EXPORT(redial, redial);


void buzzer_onoff(u8 in)
{

    GPIO_InitTypeDef GPIO_InitStructure;

    if(0 == in)
    {
        GPIO_StructInit(&GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 			//指定复用引脚
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		//模式为输入
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//频率为快速
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;		//下拉以便节省电能
        GPIO_Init(GPIOA, &GPIO_InitStructure);

    }

    if(1 == in)
    {
        //-----------------  hardware  0x101    5   Beep -----------------
        /*仅设置结构体中的部分成员：这种情况下，用户应当首先调用函数PPP_SturcInit(..)
        来初始化变量PPP_InitStructure，然后再修改其中需要修改的成员。这样可以保证其他
        成员的值（多为缺省值）被正确填入。
         */

        GPIO_StructInit(&GPIO_InitStructure);

        /*配置GPIOA_Pin_5，作为TIM2_Channel1 PWM输出*/
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 			//指定复用引脚
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		//模式必须为复用！
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//频率为快速
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		//上拉与否对PWM产生无影响
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        //GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2); //复用GPIOA_Pin1为TIM2_Ch2
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_TIM2); //复用GPIOA_Pin5为TIM2_Ch1,
    }


}
//FINSH_FUNCTION_EXPORT(buzzer_onoff, buzzer_onoff[1|0]);


void  Lora_Gprs_Send(u8 *instr, u16 inlen)
{
  GPRS_infoWr_Tx = 0;
  memcpy(GPRS_info + GPRS_infoWr_Tx,instr,inlen);
  WatchDog_Feed();
  Gsm_rxAppData_SemRelease(GPRS_info, GPRS_infoWr_Tx, LinkNum);
}

// C.  Module
