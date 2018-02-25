#include <stdio.h>

#include "Menu_Include.h"
#include <string.h>

#include "stm32f2xx.h"
unsigned char XinhaoStatus[20] = {"信号线状态:00000000"};
unsigned char XinhaoStatusBAK[20] = {"信号线状态:00000000"};


unsigned int  tzxs_value = 6000;
unsigned char send_data[10];
MB_SendDataType mb_senddata;

unsigned int CounterBack = 0;
unsigned char UpAndDown = 1; //参数设置主菜单选择序号

unsigned char Dis_date[22] = {"2000-00-00  00:00:00"}; //20
unsigned char Dis_speDer[20] ={" 000km/h            "}; //{" 000km/h    000 度"};

unsigned char GPS_Flag = 0, Gprs_Online_Flag = 0; //记录gps gprs状态的标志


unsigned char speed_time_rec[15][6];//年  月  日  时  分  速度
unsigned char ServiceNum[13];//设备的唯一性编码,IMSI号码的后12位

unsigned char KeyValue = 0;
u16  KeyCheck_Flag[6] = {0, 0, 0, 0,0,0};

unsigned char ErrorRecord = 0; //疲劳超速记录   疲劳时间错误为1超速时间错误为2,按任意键清0
PilaoRecord PilaoJilu[12];
ChaosuRecord ChaosuJilu[20];

unsigned char StartDisTiredExpspeed = 0; //开始显示疲劳或者超速驾驶的记录,再判断提示时间信息错误时用
unsigned char tire_Flag = 0, expsp_Flag = 0; //疲劳驾驶/超速驾驶  有记录为1(显示有几条记录)，无记录为2，查看记录变为3(显示按down逐条查看)
unsigned char pilaoCounter = 0, chaosuCounter = 0; //记录返回疲劳驾驶和超速驾驶的条数
unsigned char pilaoCouAscii[2], chaosuCouAscii[2];
DispMailBoxInfor LCD_Post, GPStoLCD, OtherToLCD, PiLaoLCD, ChaoSuLCD;

unsigned char SetVIN_NUM = 1; // :设置车牌号码  2:设置VIN
unsigned char OK_Counter = 0; //记录在快捷菜单下ok键按下的次数
unsigned char Screen_In = 0, Screen_in0Z = 0; //记录备选屏内选中的汉字

unsigned char OKorCancel = 1, OKorCancel2 = 1, OKorCancelFlag = 1;
unsigned char SetTZXSFlag = 0, SetTZXSCounter = 0; //SetTZXSFlag  1:校准车辆特征系数开始  2:校准车辆特征系数结束
//    1数据导出(... .../完成)  2:usb设备拔出
unsigned char OUT_DataCounter = 0; //指定导出数据类型  1、2、3
unsigned char DataOutStartFlag = 0; //数据导出标志
unsigned char DataOutOK = 0;

unsigned char Rx_TZXS_Flag = 0;

unsigned char battery_flag = 0, tz_flag = 0;
unsigned char USB_insertFlag = 1;


unsigned char BuzzerFlag = 0; //=1响1声  ＝11响2声

unsigned char DaYin = 0; //待机按下打印键为101，2s后为1，开始打印(判断是否取到数据，没有提示错误，渠道数据打印)
unsigned char DaYinDelay = 0;

unsigned char FileName_zk[11];

//==============12*12========读字库中汉字的点阵==========
unsigned char test_00[24], Read_ZK[24];
unsigned char DisComFlag = 0;
unsigned char ICcard_flag = 0;



unsigned char DisInfor_Menu[8][20];
unsigned char DisInfor_Affair[8][20];

unsigned char UpAndDown_nm[4] = {0xA1, 0xFC, 0xA1, 0xFD}; //↑ ↓

//========================================================================
unsigned char UpdataDisp[8] = {"001/000"}; //北斗升级进度
unsigned char BD_updata_flag = 0; //北斗升级度u盘文件的标志
unsigned int  FilePageBD_Sum = 0; //记录文件大小，读文件大小/514
unsigned int  bd_file_exist = 0; //读出存在要升级的文件
unsigned char device_version[30] = {"主机版本:705gghyptV2"}; //{"主机版本:V BD 2.00"};   // 北斗货运平台对接
unsigned char bd_version2[20] = {"模块版本:V 20.00.003"};



unsigned char ISP_Updata_Flag = 0; //远程升级主机程序进度显示标志   1:开始升级  2:升级完成

unsigned char data_tirexps[120];
unsigned char OneKeyCallFlag = 0; //  一键拨号
unsigned char BD_upgrad_contr = 0; //  北斗升级控制
unsigned char print_rec_flag = 0; // 打印记录标志

u8  MenuIdle_working = 0; //   Idle   界面工作状态 idle下为    1  其他为0
u8  print_workingFlag = 0; // 打印进行中。。

u8 CarSet_0_counter = 0; //记录设置车辆信息的设置内容1:车牌号2:类型3:颜色

//------------ 使用前锁定相关 ------------------
unsigned char Menu_Car_license[10];//存放车牌号码
u8 Menu_VechileType[10] = "大型车"; //  车辆类型
u8 Menu_VecLogoColor[10]; // 车牌颜色
u8 Menu_color_num = 0; // JT415    1  蓝 2 黄 3 黑 4 白 9其他
u8 Menu_Vin_Code[17];
u8 Menu_sim_Code[12];//扬州要求设置11位手机号码
u8 License_Not_SetEnable = 1; //   1:车牌号未设置
u8 menu_type_flag = 0, menu_color_flag = 0;

u8 NET_SET_FLAG = 0;
u8 CAR_SET_FLAG = 0;
/*//存储输入的相应信息
u8 Menu_MainDns[20];
u8 Menu_AuxDns[20];
u8 Menu_MainIp[20]={"   .   .   .   :    "};//000.000.000.000:0000   20位
u8 Menu_AuxIp[20]={"   .   .   .   :    "};//000.000.000.000:0000   20位;
u8 Menu_Apn[20];
*/

u8 Password_correctFlag = 0; // 密码正确
u8 Exit_to_Idle = 0;
u8 Dis_deviceid_flag = 0;
u8 MENU_set_carinfor_flag = 0; //菜单进入单项设置车辆信息


//============================================
unsigned char  Dis_speed_sensor[19] = {"传感器速度:   KM/H"};
unsigned char  Dis_speed_gps_bd[19] = {"GPS/BD速度:   KM/H"};
unsigned char  Dis_speed_pulse[15] = {"脉冲系数:00000"};

unsigned char  Mileage_02_05_flag = 0; //共用里程查看时区分是查看还是记录仪   1:记录仪
unsigned char  self_checking_PowerCut = 0; //自检是外电0，检测到电池供电后改为1.
unsigned char  self_checking_Antenna = 0; //自检是外电0，检测到电池供电后改为1.
unsigned char  self_checking_result = 1; //1:自检正常   2:自检异常 3: IC卡读取失败

//=========安全警示显示标识====================
u8 OverTime_before = 0; //超时前30min需要显示提示信息的标识
u8 OverTime_after = 0; //超时后30min需要显示提示信息的标识
u8 OverTime_before_Nobody = 0; //没有驾驶人,超时前30min需要显示提示信息的标识

u8 OverSpeed_approach = 0; //超速接近
u8 OverSpeed_flag = 0; //超速标识
u8 SpeedStatus_abnormal = 0; //速度状态异常

u8 Menu_txt_state = 0; //  缺纸 1   IC卡不匹配2   非标准IC卡 3    USB 所有数据导出4

u8 Menu_Number=0;  // 界面 编号  1: idle  2: handle  3: detect 4: Txt

ALIGN(RT_ALIGN_SIZE)
MENUITEM *pMenuItem;


void convert_speed_pulse(u16 spe_pul)
{


}


//中心下发消息或者条件触发显示消息函数
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

