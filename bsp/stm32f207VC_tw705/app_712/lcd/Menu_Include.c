#include <stdio.h>

#include "Menu_Include.h"
#include <string.h>

#include "stm32f2xx.h"
unsigned char XinhaoStatus[20] = {"�ź���״̬:00000000"};
unsigned char XinhaoStatusBAK[20] = {"�ź���״̬:00000000"};


unsigned int  tzxs_value = 6000;
unsigned char send_data[10];
MB_SendDataType mb_senddata;

unsigned int CounterBack = 0;
unsigned char UpAndDown = 1; //�����������˵�ѡ�����

unsigned char Dis_date[22] = {"2000-00-00  00:00:00"}; //20
unsigned char Dis_speDer[20] ={" 000km/h            "}; //{" 000km/h    000 ��"};

unsigned char GPS_Flag = 0, Gprs_Online_Flag = 0; //��¼gps gprs״̬�ı�־


unsigned char speed_time_rec[15][6];//��  ��  ��  ʱ  ��  �ٶ�
unsigned char ServiceNum[13];//�豸��Ψһ�Ա���,IMSI����ĺ�12λ

unsigned char KeyValue = 0;
u16  KeyCheck_Flag[6] = {0, 0, 0, 0,0,0};

unsigned char ErrorRecord = 0; //ƣ�ͳ��ټ�¼   ƣ��ʱ�����Ϊ1����ʱ�����Ϊ2,���������0
PilaoRecord PilaoJilu[12];
ChaosuRecord ChaosuJilu[20];

unsigned char StartDisTiredExpspeed = 0; //��ʼ��ʾƣ�ͻ��߳��ټ�ʻ�ļ�¼,���ж���ʾʱ����Ϣ����ʱ��
unsigned char tire_Flag = 0, expsp_Flag = 0; //ƣ�ͼ�ʻ/���ټ�ʻ  �м�¼Ϊ1(��ʾ�м�����¼)���޼�¼Ϊ2���鿴��¼��Ϊ3(��ʾ��down�����鿴)
unsigned char pilaoCounter = 0, chaosuCounter = 0; //��¼����ƣ�ͼ�ʻ�ͳ��ټ�ʻ������
unsigned char pilaoCouAscii[2], chaosuCouAscii[2];
DispMailBoxInfor LCD_Post, GPStoLCD, OtherToLCD, PiLaoLCD, ChaoSuLCD;

unsigned char SetVIN_NUM = 1; // :���ó��ƺ���  2:����VIN
unsigned char OK_Counter = 0; //��¼�ڿ�ݲ˵���ok�����µĴ���
unsigned char Screen_In = 0, Screen_in0Z = 0; //��¼��ѡ����ѡ�еĺ���

unsigned char OKorCancel = 1, OKorCancel2 = 1, OKorCancelFlag = 1;
unsigned char SetTZXSFlag = 0, SetTZXSCounter = 0; //SetTZXSFlag  1:У׼��������ϵ����ʼ  2:У׼��������ϵ������
//    1���ݵ���(... .../���)  2:usb�豸�γ�
unsigned char OUT_DataCounter = 0; //ָ��������������  1��2��3
unsigned char DataOutStartFlag = 0; //���ݵ�����־
unsigned char DataOutOK = 0;

unsigned char Rx_TZXS_Flag = 0;

unsigned char battery_flag = 0, tz_flag = 0;
unsigned char USB_insertFlag = 1;


unsigned char BuzzerFlag = 0; //=1��1��  ��11��2��

unsigned char DaYin = 0; //�������´�ӡ��Ϊ101��2s��Ϊ1����ʼ��ӡ(�ж��Ƿ�ȡ�����ݣ�û����ʾ�����������ݴ�ӡ)
unsigned char DaYinDelay = 0;

unsigned char FileName_zk[11];

//==============12*12========���ֿ��к��ֵĵ���==========
unsigned char test_00[24], Read_ZK[24];
unsigned char DisComFlag = 0;
unsigned char ICcard_flag = 0;



unsigned char DisInfor_Menu[8][20];
unsigned char DisInfor_Affair[8][20];

unsigned char UpAndDown_nm[4] = {0xA1, 0xFC, 0xA1, 0xFD}; //�� ��

//========================================================================
unsigned char UpdataDisp[8] = {"001/000"}; //������������
unsigned char BD_updata_flag = 0; //����������u���ļ��ı�־
unsigned int  FilePageBD_Sum = 0; //��¼�ļ���С�����ļ���С/514
unsigned int  bd_file_exist = 0; //��������Ҫ�������ļ�
unsigned char device_version[30] = {"�����汾:705gghyptV2"}; //{"�����汾:V BD 2.00"};   // ��������ƽ̨�Խ�
unsigned char bd_version2[20] = {"ģ��汾:V 20.00.003"};



unsigned char ISP_Updata_Flag = 0; //Զ�������������������ʾ��־   1:��ʼ����  2:�������

unsigned char data_tirexps[120];
unsigned char OneKeyCallFlag = 0; //  һ������
unsigned char BD_upgrad_contr = 0; //  ������������
unsigned char print_rec_flag = 0; // ��ӡ��¼��־

u8  MenuIdle_working = 0; //   Idle   ���湤��״̬ idle��Ϊ    1  ����Ϊ0
u8  print_workingFlag = 0; // ��ӡ�����С���

u8 CarSet_0_counter = 0; //��¼���ó�����Ϣ����������1:���ƺ�2:����3:��ɫ

//------------ ʹ��ǰ������� ------------------
unsigned char Menu_Car_license[10];//��ų��ƺ���
u8 Menu_VechileType[10] = "���ͳ�"; //  ��������
u8 Menu_VecLogoColor[10]; // ������ɫ
u8 Menu_color_num = 0; // JT415    1  �� 2 �� 3 �� 4 �� 9����
u8 Menu_Vin_Code[17];
u8 Menu_sim_Code[12];//����Ҫ������11λ�ֻ�����
u8 License_Not_SetEnable = 1; //   1:���ƺ�δ����
u8 menu_type_flag = 0, menu_color_flag = 0;

u8 NET_SET_FLAG = 0;
u8 CAR_SET_FLAG = 0;
/*//�洢�������Ӧ��Ϣ
u8 Menu_MainDns[20];
u8 Menu_AuxDns[20];
u8 Menu_MainIp[20]={"   .   .   .   :    "};//000.000.000.000:0000   20λ
u8 Menu_AuxIp[20]={"   .   .   .   :    "};//000.000.000.000:0000   20λ;
u8 Menu_Apn[20];
*/

u8 Password_correctFlag = 0; // ������ȷ
u8 Exit_to_Idle = 0;
u8 Dis_deviceid_flag = 0;
u8 MENU_set_carinfor_flag = 0; //�˵����뵥�����ó�����Ϣ


//============================================
unsigned char  Dis_speed_sensor[19] = {"�������ٶ�:   KM/H"};
unsigned char  Dis_speed_gps_bd[19] = {"GPS/BD�ٶ�:   KM/H"};
unsigned char  Dis_speed_pulse[15] = {"����ϵ��:00000"};

unsigned char  Mileage_02_05_flag = 0; //������̲鿴ʱ�����ǲ鿴���Ǽ�¼��   1:��¼��
unsigned char  self_checking_PowerCut = 0; //�Լ������0����⵽��ع�����Ϊ1.
unsigned char  self_checking_Antenna = 0; //�Լ������0����⵽��ع�����Ϊ1.
unsigned char  self_checking_result = 1; //1:�Լ�����   2:�Լ��쳣 3: IC����ȡʧ��

//=========��ȫ��ʾ��ʾ��ʶ====================
u8 OverTime_before = 0; //��ʱǰ30min��Ҫ��ʾ��ʾ��Ϣ�ı�ʶ
u8 OverTime_after = 0; //��ʱ��30min��Ҫ��ʾ��ʾ��Ϣ�ı�ʶ
u8 OverTime_before_Nobody = 0; //û�м�ʻ��,��ʱǰ30min��Ҫ��ʾ��ʾ��Ϣ�ı�ʶ

u8 OverSpeed_approach = 0; //���ٽӽ�
u8 OverSpeed_flag = 0; //���ٱ�ʶ
u8 SpeedStatus_abnormal = 0; //�ٶ�״̬�쳣

u8 Menu_txt_state = 0; //  ȱֽ 1   IC����ƥ��2   �Ǳ�׼IC�� 3    USB �������ݵ���4

u8 Menu_Number=0;  // ���� ���  1: idle  2: handle  3: detect 4: Txt

ALIGN(RT_ALIGN_SIZE)
MENUITEM *pMenuItem;


void convert_speed_pulse(u16 spe_pul)
{


}


//�����·���Ϣ��������������ʾ��Ϣ����
void Cent_To_Disp(void)
{

}
void version_disp(u8 value)
{


}
//  0   1            34             67
//(3)  1 [2-33]   2 [35-66]   3 [68-99]

void ReadEXspeed(unsigned char NumExspeed)
{


}

void Dis_chaosu(unsigned char *p)
{

}


void Show_Menu_5_2_ExportData( void )
{

}

