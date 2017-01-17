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
u8 Setting08[80] = "Ԥ�� 	    ����  	   ���  	   �����	   Զ���	   ��ת��	   ��ת��	   ɲ�� 	    ";




//------ Photo -----
u32 PicFileSize = 0; // ͼƬ�ļ���С
u8  PictureName[40];



//------  voice -----



//------  video  --------


/*
             ��
*/
//------ phone
u8       CallState = CallState_Idle; // ͨ��״̬

//   ASCII  to   GB    ---- start
//0-9        10
u8  arr_A3B0[20] = {0xA3, 0xB0, 0xA3, 0xB1, 0xA3, 0xB2, 0xA3, 0xB3, 0xA3, 0xB4, 0xA3, 0xB5, 0xA3, 0xB6, 0xA3, 0xB7, 0xA3, 0xB8, 0xA3, 0xB9};

//@ A-O      16
u8  arr_A3C0[32] = {0xA3, 0xC0, 0xA3, 0xC1, 0xA3, 0xC2, 0xA3, 0xC3, 0xA3, 0xC4, 0xA3, 0xC5, 0xA3, 0xC6, 0xA3, 0xC7, 0xA3, 0xC8, 0xA3, 0xC9, 0xA3, 0xCA, 0xA3, 0xCB, 0xA3, 0xCC, 0xA3, 0xCD, 0xA3, 0xCE, 0xA3, 0xCF};

//P-Z         11��
u8  arr_A3D0[22] = {0xA3, 0xD0, 0xA3, 0xD1, 0xA3, 0xD2, 0xA3, 0xD3, 0xA3, 0xD4, 0xA3, 0xD5, 0xA3, 0xD6, 0xA3, 0xD7, 0xA3, 0xD8, 0xA3, 0xD9, 0xA3, 0xDA};

//.  a-0       16
u8  arr_A3E0[32] = {0xA3, 0xE0, 0xA3, 0xE1, 0xA3, 0xE2, 0xA3, 0xE3, 0xA3, 0xE4, 0xA3, 0xE5, 0xA3, 0xE6, 0xA3, 0xE7, 0xA3, 0xE8, 0xA3, 0xE9, 0xA3, 0xEA, 0xA3, 0xEB, 0xA3, 0xEC, 0xA3, 0xED, 0xA3, 0xEE, 0xA3, 0xEF};

//p-z          11
u8  arr_A3F0[22] = {0xA3, 0xF0, 0xA3, 0xF1, 0xA3, 0xF2, 0xA3, 0xF3, 0xA3, 0xF4, 0xA3, 0xF5, 0xA3, 0xF6, 0xA3, 0xF7, 0xA3, 0xF8, 0xA3, 0xF9, 0xA3, 0xFA};
//-------  ASCII to GB ------



//----------- �г���¼�����  -----------------
u8          Vehicle_sensor = 0; // ����������״̬   0.2s  ��ѯһ��
u8          Vehicle_sensor_Bak=0;  // BAK
/*
D7  ɲ��
D6  ��ת��
D5  ��ת��
D4  ����
D3  Զ���
D2  ��ˢ
D1  Ԥ��
D0  Ԥ��
*/

u8          save_sensorCounter = 0, sensor_writeOverFlag = 0;;


u8       DispContent = 0; // ����ʱ�Ƿ���ʾ��������
/*
            1 <->  ������ʾ
            2 <->  ��ʾ������Ϣ��
            3 <->  ��ʾ ������������
            0<-> ����ʾ���������ֻ��ʾЭ������
     */

u8         TextInforCounter = 0; //�ı���Ϣ����

u8 		   FCS_GPS_UDP = 0;						//UDP ��������
u8         FCS_RX_UDP = 0;                     // UDP ���ݽ���У��
u8         FCS_error_counter = 0;              // У����������

u8          Centre_IP_modify = 0;             //  ���޸�IP��
u8          IP_change_counter = 0;           //   �����޸�IP ������
u8          Down_Elec_Flag = 0;              //   ���Ͷϵ�ʹ�ܱ�־λ



//---------74CH595  Q5   control Power----
u8   Print_power_Q5_enable = 0;
u8   Q7_enable = 0;







//------------ ���ٱ���---------------------
SPD_EXP speed_Exd;


//--------  GPS prototcol----------------------------------------------------------------------------------
static u32 	fomer_time_seconds, tmp_time_secnonds, delta_time_seconds;
u8	        UDP_dataPacket_flag = 'V';			  /*V	   0X03 	 ;		   A	  0X02*/
u8          Year_illigel = 0; //  ��ݲ��Ϸ�
u8	        GPS_getfirst = 0; 		 //  �״��о�γ��
u8          HDOP_value = 99;       //  Hdop ��ֵ
u8          Satelite_num = 0; // ���ǿ���
u8 CurrentTime[3];
u8 BakTime[3];
u8 Sdgps_Time[3];  // GPS ���� ʱ���¼   BCD ��ʽ

//u16  Spd_add_debug=0;

//static u8      UDP_AsciiTx[1800];
ALIGN(RT_ALIGN_SIZE)
u8      GPRS_info[1400];
u16     GPRS_infoWr_Tx = 0;

ALIGN(RT_ALIGN_SIZE)
u8  UDP_HEX_Rx[1024];    // EM310 ��������hex

u16 UDP_hexRx_len = 0;  // hex ���� ����
u16 UDP_DecodeHex_Len = 0; // UDP���պ�808 ���������ݳ���


GPS_RMC GPRMC; // GPMC��ʽ

/*                         pGpsRmc->status,\
						pGpsRmc->latitude_value,\
						pGpsRmc->latitude,\
						pGpsRmc->longtitude_value,\
						pGpsRmc->longtitude,\
						pGpsRmc->speed,\
						pGpsRmc->azimuth_angle);
						*/



//----------808 Э�� -------------------------------------------------------------------------------------
u16	   GPS_Hight = 0;             //   808Э��-> �߳�   m
u16     GPS_direction = 0;         //   808Э��-> ����   ��
u16     Centre_FloatID = 0; //  ������Ϣ��ˮ��
u16     Centre_CmdID = 0; //  ��������ID

u8      Original_info[1400]; // û��ת�崦��ǰ��ԭʼ��Ϣ
u16     Original_info_Wr = 0; // ԭʼ��Ϣд��ַ
//---------- ��GPSУ׼����ϵ����� ----------------------------
u8      Speed_area = 60; // У��Kֵ��Χ
u16	    Spd_Using = 0;			 //   808Э��-> �ٶ�   0.1km/h      ��ǰʹ�õ��ٶȣ��жϳ���ƣ�͵�����
u32     Sps_larger_5_counter = 0;  //   GPS  using  ����   5km/h  ������
u16     Speed_gps = 0; // ͨ��GPS����������ٶ� 0.1km/h
u16     Speed_jiade = 0; //  �ٵ��ٶ�   1: enable 0: disable
u8      Speed_Rec = 0; // �ٶȴ����� У��K�õĴ洢��
u16     Speed_cacu = 0; // ͨ��Kֵ����������ٶ�    ͨ����������ȡ���ٶ�
u16     Speed_cacu_BAK = 0; //  ������  ����
u8      Speed_cacu_Trigger_Flag = 0;
u16     Spd_adjust_counter = 0; // ȷ������״̬������
u16     Spd_Deltacheck_counter = 0; // �������ٶȺ������ٶ����ϴ��ж�
u16     Former_DeltaPlus[K_adjust_Duration]; // ǰ�����������
u8      Former_gpsSpd[K_adjust_Duration];// ǰ������ٶ�
u8      Illeagle_Data_kickOUT = 0; //  �޳��Ƿ�����״̬

//-----  ��̨ע�ᶨʱ��  ----------
DevRegst   DEV_regist;  // ע��
DevLOGIN   DEV_Login;   //  ��Ȩ




//------- �ı���Ϣ�·� -------
TEXT_INFO      TextInfo;    // �ı���Ϣ�·�
//-------�ı���Ϣ-------
MSG_TEXT       TEXT_Obj;
MSG_TEXT       TEXT_Obj_8[8], TEXT_Obj_8bak[8];

//------ ����  --------
CENTRE_ASK     ASK_Centre;  // ��������

//-----  �������� ------
VEHICLE_CONTROL Vech_Control; //  ��������
//-------    �г���¼��  -----
RECODER      Recode_Obj;     // �г���¼��
//-------  ����  ----
CAMERA        Camera_Obj;     //  �����������
//-----   ¼��  ----
VOICE_RECODE  VoiceRec_Obj;   //  ¼������
//------ ��ý��  --------
MULTIMEDIA    MediaObj;       // ��ý����Ϣ
//-------  ������Ϣ͸��  -------
DATATRANS     DataTrans;      // ������Ϣ͸��
//-------  ����Χ��״̬ --------
INOUT        InOut_Object;    // ����Χ��״̬
//-------- ��ý�����  ------------
MEDIA_INDEX  MediaIndex;  // ��ý����Ϣ
//------- ��������״̬ ---------------
u8  CarLoadState_Flag = 1; //ѡ�г���״̬�ı�־   1:�ճ�   2:���   3:�س�

//------- ��ý����Ϣ����---------------
u8  Multimedia_Flag = 1; //��Ҫ�ϴ��Ķ�ý����Ϣ����   1:��Ƶ   2:��Ƶ   3:ͼ��
u8  SpxBuf[SpxBuf_Size];
u16 Spx_Wr = 0, Spx_Rd = 0;
u8  Duomeiti_sdFlag = 0;

//------- ¼����ʼ���߽���---------------
u8  Recor_Flag = 1; //  1:¼����ʼ   2:¼������

//----------808Э�� -------------------------------------------------------------------------------------
u8		SIM_code[6];							   // Ҫ���͵�IMSI	����
u8		IMSI_CODE[15] = "000000000000000";							//SIM ����IMSI ����
u8		Warn_Status[4]		=
{
    0x00, 0x00, 0x00, 0x00
}; //  ������־λ״̬��Ϣ
u8  Warn_MaskWord[4]		=
{
    0x00, 0x00, 0x00, 0x00
};   //  ����������
u8  Text_MaskWord[4] =
{
    0x00, 0x00, 0x00, 0x00
};	 //  �ı�������
u8  Key_MaskWord[4] =
{
    0x00, 0x00, 0x00, 0x00
};	 //   �ؼ���������



u8		Car_Status[4]		=
{
    0x00, 0x0c, 0x00, 0x00
}; //  ����״̬��Ϣ
T_GPS_Info_GPRS 	Gps_Gprs, Bak_GPS_gprs;
T_GPS_Info_GPRS	Temp_Gps_Gprs;
u8   A_time[6]; // ��λʱ�̵�ʱ��

u8      ReadPhotoPageTotal = 0;
u8      SendPHPacketFlag = 0; ////�յ���������������һ��blockʱ��λ


//-------- �������� --------
u8		warn_flag = 0;
u8		f_Exigent_warning = 0; //0;     //�Ŷ� ��������װ�� (INT0 PD0)
u8		Send_warn_times = 0;    //   �豸�������ϱ��������������3 ��
u32  	fTimer3s_warncount = 0;

// ------  ������Ϣ������ ---------------
VechINFO     Vechicle_Info;     //  ������Ϣ
VechINFO     Vechicle_Info_BAK;  //  ������Ϣ BAK
VechINFO     Vechicle_info_BAK2; //  ������ϢBAK2
u8           Login_Menu_Flag = 1;     //   ��½���� ��־λ
u8           Limit_max_SateFlag = 0;  //   �ٶ������������ָ��


//------  ���ſ������� -------
DOORCamera   DoorOpen;    //  ���س�������

//------- ������չЭ��  ------------
BD_EXTEND     BD_EXT;     //  ������չЭ��
DETACH_PKG   Detach_PKG; // �ְ��ش����
SET_QRY         Setting_Qry; //  �ն˲�����ѯ
u32     CMD_U32ID = 0;
PRODUCT_ATTRIBUTE   ProductAttribute;// �ն�����
HUMAN_CONFIRM_WARN   HumanConfirmWarn;// �˹�ȷ�ϱ���

//-----  ISP    Զ��������� -------
ISP_BD  BD_ISP; //  BD   ������



// ---- �յ� -----
u16  Inflexion_Current = 0;
u16  Inflexion_Bak = 0;
u16  Inflexion_chgcnter = 0; //�仯������
u16  InflexLarge_or_Small = 0;   // �ж�curent �� Bak ��С    0 equql  1 large  2 small
u16  InflexDelta_Accumulate = 0; //  ��ֵ�ۼ�

// ----����״̬  ------------
u8  SleepState = 0; //   0  ������ACC on            1  ����Acc Off
u8  SleepConfigFlag = 0; //  ����ʱ���ͼ�Ȩ��־λ

//---- �̶��ļ���С ---
u32 mp3_fsize = 5616;
u8  mp3_sendstate = 0;
u32 wmv_fsize = 25964;
u8  wmv_sendstate = 0;

//-------------------   ���� ---------------------------------------
static u8 GPSsaveBuf[128];     // �洢GPS buffer
static u8	ISP_buffer[1024];
static u16 GPSsaveBuf_Wr = 0;


POSIT Posit[60];           // ÿ����λ����Ϣ�洢
u8    PosSaveFlag = 0;    // �洢Pos ״̬λ

NANDSVFlag   NandsaveFlg;
A_AckFlag    Adata_ACKflag;  // ����GPRSЭ�� ������� RS232 Э�鷵��״̬�Ĵ���
TCP_ACKFlag  SD_ACKflag;     // ����GPRSЭ�鷵��״̬��־
u32  SubCMD_8103H = 0;          //  02 H���� ���ü�¼�ǰ�װ�����ظ� ������
u32  SubCMD_FF01H = 0;          //  FF02 ������Ϣ��չ
u32  SubCMD_FF03H = 0;   //  FF03  ������չ�ն˲�������1
u8   Fail_Flag = 0;


u8  SubCMD_10H = 0;          //  10H   ���ü�¼�Ƕ�λ�澯����
u8  OutGPS_Flag = 0;   //  0  Ĭ��  1  ���ⲿ��Դ����
u8   Spd_senor_Null = 0; // �ֶ��������ٶ�Ϊ0
u32  Centre_DoubtRead = 0;   //  ���Ķ�ȡ�¹��ɵ����ݵĶ��ֶ�
u32  Centre_DoubtTotal = 0;  //  ���Ķ�ȡ�¹��ɵ�����ֶ�
u8   Vehicle_RunStatus = 0;  //  bit 0: ACC �� ��             1 ��  0��
//  bit 1: ͨ���ٶȴ�������֪    1 ��ʾ��ʻ  0 ��ʾֹͣ
//  bit 2: ͨ��gps�ٶȸ�֪       1 ��ʾ��ʻ  0 ��ʾֹͣ



u32   SrcFileSize = 0, DestFilesize = 0, SrcFile_read = 0;
u8    SleepCounter = 0;

u16   DebugSpd = 0; //������GPS�ٶ�
u8    MMedia2_Flag = 0; // �ϴ�������Ƶ ��ʵʱ��Ƶ  �ı�־λ    0 ������ 1 ��ʵʱ


u8	 reg_128[128];  // 0704 �Ĵ���

unsigned short int FileTCB_CRC16 = 0;
unsigned short int Last_crc = 0, crc_fcs = 0;



//---------  ����Ӧ��  -----------
u8		 ACK_timer = 0;				 //---------	ACK timer ��ʱ��---------------------
u8           Send_Rdy4ok = 0;
unsigned char	Rstart_time = 0;

u8   Flag_0200_send=0; // ����0200  flag
u16  Timer_0200_send=0; // 0200  �ж�Ӧ��


//---------------  �ٶ��������-------------- 
u32  Delta_1s_Plus = 0;
u16  Sec_counter = 0;
u32  TimeTriggerPhoto_counter = 0; // ��ʱ�������ռ�ʱ��
u32  Timer_stop_taking_timer=0;  //   ��ֹ��ʱ���� ��ʱ�� 
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
// �������� :   ������GPRS���ߴ������Э��
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
    //---�ж��ļ����±�־---------------------
    if(ISP_buffer[32] != ISP_BYTE_CrcPass) //  ����У��ͨ����  ���±�־�ĳ�0xE1     ��ǰ��0xF1
    {
        rt_kprintf("\r\n �����ͺ���ȷ\r\n");
        return;
    }

    /*
       ���   �ֽ���	����			  ��ע
      1 		  1    ���±�־ 	 1 ��ʾ��Ҫ����   0 ��ʾ����Ҫ����
      2-5			  4   �豸����				 0x0000 0001  ST712   TWA1
    									0x0000 0002   STM32  103  ��A1
    									0x0000 0003   STM32  101  ������
    									0x0000 0004   STM32  A3  sst25
    									0x0000 0005   STM32  �г���¼��
      6-9		 4	   ����汾 	 ÿ���豸���ʹ�  0x0000 00001 ��ʼ���ݰ汾���ε���
      10-29 	  20	����		' mm-dd-yyyy HH:MM:SS'
      30-31 	  2    ��ҳ��		   ��������Ϣҳ
      32-35 	  4    ������ڵ�ַ
      36-200	   165	  Ԥ��
      201-		  n    �ļ���

    */
    //------------   Type check  ---------------------
    memset(FileHead, 0, sizeof(FileHead));
    memcpy(FileHead, ISP_buffer, 32);
    rt_kprintf( "\r\n FileHead:%s\r\n", FileHead );


    //------    �ļ���ʽ�ж�
    if(strncmp(ISP_buffer + 32 + 13, "70420TW705", 10) == 0) //�жϳ��̺��ͺ�
    {
        ISP_judge_resualt++;// step 1
        rt_kprintf("\r\n �����ͺ���ȷ\r\n");

        // hardware
        HardVersion = (ISP_buffer[32 + 38] << 24) + (ISP_buffer[32 + 39] << 16) + (ISP_buffer[32 + 40] << 8) + ISP_buffer[32 + 41];
        HardWareVerion = HardWareGet();
        if(HardWareVerion == HardVersion)	// Ҫ������ǰ���ϰ��� ȫ1
        {
            ISP_judge_resualt++;// step 2
            rt_kprintf("\r\n Ӳ���汾:%d\r\n", HardVersion);
        }
        else
            rt_kprintf("\r\n Ӳ���汾��ƥ��!\r\n");
        //firmware
        if(strncmp((const char *)ISP_buffer + 32 + 42, "NXBXGGHYPT", 10) == 0)
        {
            ISP_judge_resualt++;// step 3
            rt_kprintf("\r\n  �̼��汾:NXBXGGHYPT\r\n");
        }
        // operater
        if(strncmp((const char *)ISP_buffer + 32 + 62, "NXBX", 4) == 0)
        {
            ISP_judge_resualt++;// step 4
            rt_kprintf("\r\n  �̼��汾:NXBX\r\n");
        }

    }

    //ISP_judge_resualt=4;
    if(ISP_judge_resualt == 4)
    {
        //------- enable  flag -------------
        SST25V_BufferRead( FileHead, 0x001000, 100 );
        FileHead[32] = ISP_BYTE_Rdy2Update;    //-----  �ļ����±�־  ʹ������ʱ����
        SST25V_BufferWrite( FileHead, 0x001000, 100);

        {
            Systerm_Reset_counter = (Max_SystemCounter - 5);	 // ׼�������������³���
            ISP_resetFlag = 1; //׼������
            rt_kprintf( "\r\n ׼���������³���!\r\n" );
        }

        // rt_kprintf( "\r\n ��������ˣ��������µȴ��ж� У����� \r\n" );
    }
    else
    {
        //------- enable  flag -------------
        SST25V_BufferRead( FileHead, 0x001000, 100 );
        FileHead[32] = ISP_BYTE_TypeNotmatch;    //-----   �ļ�У��ͨ������ ���Ͳ�ƥ��
        SST25V_BufferWrite( FileHead, 0x001000, 100);
        BD_ISP.ISP_running = 0; // recover normal
        rt_kprintf( "\r\n ��ز�����ƥ�䲻�����!\r\n" );
    }

}
// FINSH_FUNCTION_EXPORT(ISP_file_Check, ISP_file_Check);

u16  Instr_2_GBK(u8 *SrcINstr, u16 Inlen, u8 *DstOutstr )
{
    u16 i = 0, j = 0;


    //�Է�GBK���봦��------------------------------------
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
void TCP_RX_Process( u8  LinkNum)  //  ---- 808  ��׼Э��
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

    //  ��ȡ  �ļ�ͷ��Ϣ
    SST25V_BufferRead( buffer_temp, ISP_StartArea,  256 );
    File_size = (buffer_temp[114] << 24) + (buffer_temp[115] << 16) + (buffer_temp[116] << 8) + buffer_temp[117];

    leftvalue = File_size % 512;
    rt_kprintf("\r\n �ļ���С: %d Bytes  leftvalue=%d \r\n", File_size, leftvalue);
    FileTCB_CRC16 = (buffer_temp[134] << 8) + buffer_temp[135];
    rt_kprintf("\r\n Read CRC16: 0x%X Bytes\r\n", FileTCB_CRC16);

    // OutPrint_HEX("1stpacket",buffer_temp,256);

    if(leftvalue)   // ���� 512
        packet_num = (File_size >> 9) + 1;
    else
        packet_num = (File_size >> 9);


    for(i = 0; i < packet_num; i++)
    {
        if(i == 0) //��һ��
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
        else if(i == (packet_num - 1)) //���һ��
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
            // �м�İ�
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
    rt_kprintf("\r\n  У���� 0x%X \r\n", crc_fcs);

    if(FileTCB_CRC16 == crc_fcs)
    {
        SST25V_BufferRead( buffer_temp, 0x001000, 100 );
        buffer_temp[32] = ISP_BYTE_CrcPass;    //-----   �ļ�У��ͨ��
        SST25V_BufferWrite( buffer_temp, 0x001000, 100);
        rt_kprintf("\r\n  У����ȷ! \r\n", crc_fcs);
        return true;
    }
    else
    {
        rt_kprintf("\r\n  У��ʧ��! \r\n", crc_fcs);
        return false;
    }
}
//FINSH_FUNCTION_EXPORT(File_CRC_Get, File_CRC_Get);


/*
    ��ӡ��� HEX ��Ϣ��Descrip : ������Ϣ ��instr :��ӡ��Ϣ�� inlen: ��ӡ����
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

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 			//ָ����������
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		//ģʽΪ����
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//Ƶ��Ϊ����
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;		//�����Ա��ʡ����
        GPIO_Init(GPIOA, &GPIO_InitStructure);

    }

    if(1 == in)
    {
        //-----------------  hardware  0x101    5   Beep -----------------
        /*�����ýṹ���еĲ��ֳ�Ա����������£��û�Ӧ�����ȵ��ú���PPP_SturcInit(..)
        ����ʼ������PPP_InitStructure��Ȼ�����޸�������Ҫ�޸ĵĳ�Ա���������Ա�֤����
        ��Ա��ֵ����Ϊȱʡֵ������ȷ���롣
         */

        GPIO_StructInit(&GPIO_InitStructure);

        /*����GPIOA_Pin_5����ΪTIM2_Channel1 PWM���*/
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 			//ָ����������
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		//ģʽ����Ϊ���ã�
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//Ƶ��Ϊ����
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		//��������PWM������Ӱ��
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        //GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2); //����GPIOA_Pin1ΪTIM2_Ch2
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_TIM2); //����GPIOA_Pin5ΪTIM2_Ch1,
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
