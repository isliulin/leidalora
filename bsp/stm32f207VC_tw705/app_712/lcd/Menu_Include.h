#ifndef _H_MENU_H_
#define _H_MENU_H_

//#include "sed1520.h"
#include <stdio.h>
#include<./App_moduleConfig.h>

#define KeyValueMenu    1
#define KeyValueOk      2
#define KeyValueUP      3
#define KeyValueDown    4

//(1) �ı���Ϣ          ��ʼPage 6800-6899   100 page
//(2) �¼�����          ��ʼPage 6900-6949    50 page
//(3) ��Ϣ�㲥�˵�����  ��ʼPage 6950-6999    50 page
#define InforStartPage_Text       6800
#define InforStartPage_Affair     6900
#define InforStartPage_Meun       6950


#define DECL_BMP(width,height,imgdata)	struct IMG_DEF BMP_##imgdata={width,height,imgdata}

#define MaxBankIdleTime  240//60s  LCD����ִ������60ms,1minû�а��������Ƴ���idle״̬


typedef void (*SHOW)(void);
typedef void (*KEYPRESS)(unsigned int);
typedef void (*TIMETICK)(unsigned int);
typedef void (*MSG)(void *p);



typedef  __packed struct _menuitem
{
    char *caption;			/*�˵����������Ϣ*/
    unsigned char len;
    SHOW show;				/*��ʾʱ���ã���ʼ����ʾ*/
    KEYPRESS keypress;		/*��������ʱ����*/
    TIMETICK timetick;		/*�����ṩϵͳtick�����緵�ش�������*/
    MSG msg;
    struct _menuitem *parent;
} MENUITEM;

typedef __packed struct _menuitem *PMENUITEM;


extern unsigned int CounterBack;
extern unsigned char UpAndDown;
extern unsigned char KeyValue;
extern u16 KeyCheck_Flag[4];


typedef __packed struct
{
    unsigned int id;
    unsigned int len;
    unsigned char *p;
} MB_SendDataType;

typedef __packed struct
{
    unsigned char Num;
    unsigned char Drver_Name[22];
    unsigned char StartTime[6];
    unsigned char EndTime[6];
} PilaoRecord;

typedef __packed struct
{
    unsigned char Num;
    unsigned char Drver_Name[22];
    unsigned char StartTime[6];
    unsigned char EndTime[6];
    unsigned char Speed;
} ChaosuRecord;

typedef __packed struct
{
    unsigned char Head[3];
    unsigned int Flag;
    unsigned int AllLen;
    unsigned char ZCFlag[2];
    //unsigned char *PInfor;
    unsigned char PInfor[200];
    unsigned char CheckOut;
    unsigned char End[3];
} DispMailBoxInfor;

extern unsigned char XinhaoStatus[20];
extern unsigned char XinhaoStatusBAK[20];

extern unsigned int  tzxs_value;
extern unsigned char send_data[10];
extern MB_SendDataType mb_senddata;

extern unsigned char Dis_date[22];
extern unsigned char Dis_speDer[20];


extern unsigned char GPS_Flag, Gprs_Online_Flag; //��¼gps gprs״̬�ı�־
extern unsigned char speed_time_rec[15][6];

extern unsigned char ErrorRecord;
extern PilaoRecord PilaoJilu[12];
extern ChaosuRecord ChaosuJilu[20];

extern DispMailBoxInfor LCD_Post, GPStoLCD, OtherToLCD, PiLaoLCD, ChaoSuLCD;

extern unsigned char StartDisTiredExpspeed;//��ʼ��ʾƣ�ͻ��߳��ټ�ʻ�ļ�¼,���ж���ʾʱ����Ϣ����ʱ��
extern unsigned char tire_Flag, expsp_Flag;
extern unsigned char pilaoCounter, chaosuCounter; //��¼����ƣ�ͼ�ʻ�ͳ��ټ�ʻ������
extern unsigned char pilaoCouAscii[2], chaosuCouAscii[2];

extern unsigned char ServiceNum[13];//�豸��Ψһ�Ա���,IMSI����ĺ�12λ
extern unsigned char DisComFlag;
extern unsigned char OneKeyCallFlag;
extern unsigned char data_tirexps[120];
extern u8 CarSet_0_counter;//��¼���ó�����Ϣ����������1:���ƺ�2:����3:��ɫ

extern u8 MENU_set_carinfor_flag;


//=========��ȫ��ʾ��ʾ��ʶ====================
extern u8 OverTime_before;//��ʱǰ30min��Ҫ��ʾ��ʾ��Ϣ�ı�ʶ
extern u8 OverTime_after; //��ʱ��30min��Ҫ��ʾ��ʾ��Ϣ�ı�ʶ
extern u8 OverTime_before_Nobody;//û�м�ʻ��,��ʱǰ30min��Ҫ��ʾ��ʾ��Ϣ�ı�ʶ

extern u8 OverSpeed_approach;//���ٽӽ�
extern u8 OverSpeed_flag;//���ٱ�ʶ
extern u8 SpeedStatus_abnormal;//�ٶ�״̬�쳣

extern u8 Menu_txt_state;	//	ȱֽ 1	 IC����ƥ��2

ALIGN(RT_ALIGN_SIZE)extern  MENUITEM    *pMenuItem;

ALIGN(RT_ALIGN_SIZE)extern  MENUITEM    Menu_TXT;



ALIGN(RT_ALIGN_SIZE)extern  MENUITEM	Menu_1_Idle;
ALIGN(RT_ALIGN_SIZE)extern  MENUITEM	Menu_1_menu;



extern unsigned char SetVIN_NUM;//   1:���ó��ƺ���  2:����VIN
extern unsigned char OK_Counter;//��¼�ڿ�ݲ˵���ok�����µĴ���
extern unsigned char Screen_In, Screen_in0Z; //��¼��ѡ����ѡ�еĺ���

extern unsigned char OKorCancel, OKorCancel2, OKorCancelFlag;
extern unsigned char SetTZXSFlag, SetTZXSCounter; //SetTZXSFlag  1:У׼��������ϵ����ʼ  2:У׼��������ϵ������

extern unsigned char OUT_DataCounter, DataOutStartFlag, DataOutOK;
extern unsigned char Rx_TZXS_Flag;
extern unsigned char battery_flag, tz_flag;
extern unsigned char USB_insertFlag;

extern unsigned char BuzzerFlag;
extern unsigned char DaYin;
extern unsigned char DaYinDelay;

extern unsigned char FileName_zk[11];
//==============12*12========���ֿ��к��ֵĵ���==========
extern unsigned char test_00[24], Read_ZK[24];


//�� ��
extern unsigned char UpAndDown_nm[4];
extern unsigned char ICcard_flag;


extern unsigned char DisInfor_Menu[8][20];
extern unsigned char DisInfor_Affair[8][20];


//========================================================================
extern unsigned char UpdataDisp[8];//������������
extern unsigned char BD_updata_flag;//����������u���ļ��ı�־
extern unsigned int  FilePageBD_Sum;//��¼�ļ���С�����ļ���С/514
extern unsigned int  bd_file_exist;//��������Ҫ�������ļ�
extern unsigned char device_version[30];


extern unsigned char ISP_Updata_Flag; //Զ�������������������ʾ��־   1:��ʼ����  2:�������

extern unsigned char BD_upgrad_contr;
extern unsigned char print_rec_flag;
extern u8 print_workingFlag;  // ��ӡ�����С���
extern u8	MenuIdle_working;  //   Idle	 ���湤��״̬ idle��Ϊ	  1  ����Ϊ0

//------------ ʹ��ǰ������� ------------------
extern unsigned char Menu_Car_license[10];//��ų��ƺ���
extern u8  Menu_VechileType[10];  //  ��������
extern u8  Menu_VecLogoColor[10]; // ������ɫ
extern u8 Menu_color_num;
extern u8 Menu_Vin_Code[17];
extern u8 Menu_sim_Code[12];
extern u8 License_Not_SetEnable;//    1:���ƺ�δ����
extern u8 Menu_color_num;
extern u8 menu_type_flag, menu_color_flag;


extern u8 Password_correctFlag;  // ������ȷ
extern u8 Exit_to_Idle;
extern u8 Dis_deviceid_flag;

extern u8 NET_SET_FLAG;
extern u8 CAR_SET_FLAG;
/*//�洢�������Ӧ��Ϣ
extern u8 Menu_MainDns[20];
extern u8 Menu_AuxDns[20];
extern u8 Menu_MainIp[20];
extern u8 Menu_AuxIp[20];
extern u8 Menu_Apn[20];
*/
//============================
extern unsigned char  Dis_speed_sensor[19];
extern unsigned char  Dis_speed_gps_bd[19];
extern unsigned char  Dis_speed_pulse[15];
extern unsigned char  Mileage_02_05_flag;  //������̲鿴ʱ�����ǲ鿴���Ǽ�¼��   1:��¼��
extern unsigned char  self_checking_PowerCut;//�Լ������0����⵽��ع�����Ϊ1.
extern unsigned char  self_checking_result;//1:�Լ�����   2:�Լ��쳣
extern unsigned char  self_checking_Antenna;//�Լ������0����⵽��ع�����Ϊ1.

extern void convert_speed_pulse(u16 spe_pul);
extern void Cent_To_Disp(void);
extern void version_disp(u8 value);
extern void ReadEXspeed(unsigned char NumExspeed);
extern void Dis_chaosu(unsigned char *p);

#endif
