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

ALIGN(RT_ALIGN_SIZE)
u8      GPRS_info[1400];
u16     GPRS_infoWr_Tx = 0;

ALIGN(RT_ALIGN_SIZE)
u8  UDP_HEX_Rx[1024];    // EM310 接收内容hex

u16 UDP_hexRx_len = 0;  // hex 内容 长度
u16 UDP_DecodeHex_Len = 0; // UDP接收后808 解码后的数据长度


u8   A_time[6]; // 定位时刻的时间

u8      ReadPhotoPageTotal = 0;
u8      SendPHPacketFlag = 0; ////收到中心启动接收下一个block时置位



u8	 reg_128[128];  // 0704 寄存器

unsigned short int FileTCB_CRC16 = 0;
unsigned short int Last_crc = 0, crc_fcs = 0;

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
    rt_kprintf( "\r\n FileHead:%s\r\n", FileHead );


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
FINSH_FUNCTION_EXPORT(buzzer_onoff, buzzer_onoff[1|0]);


void  Lora_Gprs_Send(u8 *instr, u16 inlen)
{
  GPRS_infoWr_Tx = 0;
  memcpy(GPRS_info + GPRS_infoWr_Tx,instr,inlen);
  WatchDog_Feed();
  Gsm_rxAppData_SemRelease(GPRS_info, GPRS_infoWr_Tx, LinkNum);
}

// C.  Module
