#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "Device_808.h"


#define   SYSID            0x8888     //55AA     A712
#define   LORA_DEFAULT_ADDR       0x1234    // 
#define   LORA_DEFAULT_CHANNEL    23       //0X19  25
#define   LORA_SYS_CHANNEL        23        //17   默认信道


/*
      主机    0x1236       0x19       25
      中继  0x1234         0x17       23  

*/
/*
                        0x0000   -----   0x00FF  生产和研发用
                        0x0100   -----   0x0FFF  产品出货用
                        0x1000   -----   0xF000  远程升级用
                       */



ALIGN(RT_ALIGN_SIZE)
SYS_CONF          SysConf_struct;   //  系统配置
SYS_CONF          SysConf_struct_BAK;   //  系统配置
SYS_CONF          SysConf_struct_BAK2;   //  系统配置


ALIGN(RT_ALIGN_SIZE)





//----------  Basic  Config---------------------------
u8      DeviceNumberID[13];//="800130100001";    // 车辆DeviceID    ---- 河北天地通用
u8      SimID_12D[13]; // 入网ID  号码 12 位 首位 为 0

u8          RemoteIP_Dnsr[4]={255,255,255,255}; 
u8		RemoteIP_main[4]={111,113,14,154};  //{111,113,14,154}; 宁夏北星         
u16		RemotePort_main= 9131;//山西中航北斗   7000
u8		RemoteIP_aux[4]={60,28,50,210};    //{60,28,50,210}
u16		RemotePort_aux=4000; 
//           Link2  Related 
u8      Remote_Link2_IP[4]={111,113,14,154}; 
u16     Remote_Link2_Port=7000;     



u8           APN_String[30] = "UNINET"; //"CMNET";   //  河北天地通  移动的卡
u8           DomainNameStr[50] = "jt1.gghypt.net"; ; // 域名  天地通up.gps960.com //jt1.gghypt.net
u8           DomainNameStr_aux[50] = "jt2.gghypt.net";   //"www.sina.com";//jt2.gghypt.net
u8          TriggerSDsatus = 0x80; // 传感器触发上报状态位


//---------  SytemCounter ------------------
u32  Systerm_Reset_counter = 0;
u8   DistanceWT_Flag = 0; //  写里程标志位
u8   SYSTEM_Reset_FLAG = 0;      // 系统复位标志位
u32  Device_type = 0x00000001; //硬件类型   STM32103  新A1
u32  Firmware_ver = 0x0000001; // 软件版本
u8   ISP_resetFlag = 0;      //远程升级复位标志位



/*
       系统配置信息写入
*/
u8  SysConfig_init(void)
{

    //  1. Stuff
    //   系统版本
    SysConf_struct.Version_ID = SYSID;	
	SysConf_struct.LORA_Baud=9600;
    SysConf_struct.LORA_Local_ADDRESS=0x0000;
	SysConf_struct.LORA_Local_Channel=0;
	SysConf_struct.LORA_dest1_ADDRESS=LORA_DEFAULT_ADDR;  
	SysConf_struct.LORA_dest1_Channel=LORA_DEFAULT_CHANNEL;
	SysConf_struct.LORA_SYS_Channel=LORA_SYS_CHANNEL;  //	LORA  系统频点
	SysConf_struct.LORA_TYPE=LORA_HANDLE_DEV;  // 终端 
	SysConf_struct.LORA_DIRECTION=0;
	SysConf_struct.RTC_updated=0;    //  RTC 校准状态标志位
	memset( SysConf_struct.LORA_PointDesc,0,sizeof( SysConf_struct.LORA_PointDesc));
	memcpy(SysConf_struct.LORA_PointDesc,"终端功能描述",12);
    

    cycle_write=0;
	cycle_read=0;
    DF_Write_RecordAdd(cycle_write, cycle_read, TYPE_CycleAdd);
	
    //    2. Operate
    return(Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct)));

}

void SysConfig_Read(void)
{
   u16   res[3];
    //读取系统配置信息
   

    DF_WP_pageRead(ConfigStart_offset,(u8 *)&SysConf_struct, sizeof(SysConf_struct));

    WatchDog_Feed();
    DF_WP_pageRead(ConfigStart_BakSetting_offset,(u8 *)&SysConf_struct_BAK, sizeof(SysConf_struct_BAK));

    WatchDog_Feed();
    DF_WP_pageRead(ConfigStart_Bak2Setting_offset,(u8 *)&SysConf_struct_BAK2, sizeof(SysConf_struct_BAK2));

    //  compare
    /*
            note:   res[0] == org cmp  bak    res[1]== bak  cmp  bak2    res[2]== bak2  cmp  org

            ---org --<seg1>--  bak ---<seg2>----bak2 ---<seg3>---
            |-----------<---------------<----------------------|
     */ 
    res[0] = memcmp((u8 *)&SysConf_struct, (u8 *)&SysConf_struct_BAK, sizeof(SysConf_struct_BAK));
    res[1] = memcmp((u8 *)&SysConf_struct_BAK, (u8 *)&SysConf_struct_BAK2, sizeof(SysConf_struct_BAK));
    res[2] = memcmp((u8 *)&SysConf_struct_BAK2, (u8 *)&SysConf_struct, sizeof(SysConf_struct_BAK));

    // 3. judge
    if(res[0] && res[1] && res[2])	 // 全有问题
    {
        rt_kprintf("\r\n SysConf_struct全部失败! \r\n");
        rt_kprintf("\r\n need all recover");
        SysConfig_init();// 写入系统配置信息
        reset(); 
    }
    else if(res[0] && res[1])	 //    seg1  seg2  有问题说明  BAK error
    {
        // org  bak2 ---ok 	 bak---error
        if((u8)(SysConf_struct.Version_ID>> 8) != 0xFF) // 判断正确的是不是 FF
        {
            DF_WP_pageWrite(ConfigStart_BakSetting_offset,(u8 *)&SysConf_struct, sizeof(SysConf_struct));
            rt_kprintf("\r\n SysConf_struct BAK error ,correct ok");
			
        }
        else
        {
            rt_kprintf("\r\n SysConf_struct need all recover 1");
            SysConfig_init();
        }		
        reset();
    }
    else if(res[0] && res[2])	//	seg1  seg3	  有问题说明 BAK2  error
    {
        // org	bak  ---ok		 bak2 -----error
        if((u8)(SysConf_struct.Version_ID >> 8) != 0xFF) // 判断正确的是不是 FF
        {
            DF_WP_pageWrite((ConfigStart_Bak2Setting_offset<<1),(u8 *)&SysConf_struct, sizeof(SysConf_struct));
            rt_kprintf("\r\n SysConf_struct BAK2 error ,correct ok");
			
        }
        else
        {
            rt_kprintf("\r\n SysConf_struct need all recover 2");
            SysConfig_init();
        }		
        reset();

    }
    else if(res[1] && res[2])	//	seg2  seg3	  有问题说明 org  error
    {
        //	bak  bak2 --ok	   org---error
        if((u8)(SysConf_struct.Version_ID >> 8) != 0xFF) // 判断正确的是不是 FF
        {
            DF_WP_pageWrite((ConfigStart_offset<<1),(u8 *)&SysConf_struct_BAK, sizeof(SysConf_struct_BAK));
            rt_kprintf("\r\n SysConf_struct BAK error ,correct ok");
        }
        else
        {
            rt_kprintf("\r\n SysConf_struct need all recover 3"); 
            SysConfig_init();
        }		
        reset(); 
    }
    else
        rt_kprintf("\r\n SysConf_struct 读取校验成功! \r\n"); 

    //--------------------------------------------------------------------------------------------------


   //  这个有用
    DF_Read_RecordAdd(cycle_write, cycle_read, TYPE_CycleAdd);
   

	// important  use it  first
	rt_device_write(&Device_UsrSerial, 0, ( const void *)res, (rt_size_t)3);    

}



//-----------------------------------------------------------------


//-----------------------------------------------------------------
void SetConfig(void)
{
    //u8 i=0;//,len_write=0;
    //	u32 j=0;

    rt_kprintf("\r\nSave Config\r\n");
    // 1.  读取config 操作      0 :成功    1 :  失败
    Api_Config_read(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
    //rt_kprintf("\r\nRead Save SYSID\r\n");
    //  2. 读取成功  ，判断  版本ID
    if(SysConf_struct.Version_ID != SYSID) //SYSID)   //  check  wether need  update  or not
    {
        rt_kprintf("\r\n ID not Equal   Saved==0x%X ,  Read==0x%X !\r\n", SYSID, SysConf_struct.Version_ID);
        SysConf_struct.Version_ID = SYSID; // update  ID

		 SysConfig_init();   //  写入系统配置信息 
    }
    else
        rt_kprintf("\r\n Config Already Exist!\r\n");
}

void ReadConfig(void)
{
    u16   res[3];

    DF_delay_ms(500);
    
    SysConfig_Read();  //读取系统配置信息 

	
	rt_kprintf("\r\n  cyc_recordMax=%d    cyc_outMax=%d",LORA_Record_MAX,LORA_LOG_OUT_MAX);
    rt_kprintf("\r\n  cycwrite=%d     cycread=%d  \r\n", cycle_write,cycle_read);
	lora_info();
}

/*
读参数配置文件
*/
void SysConfiguration(void)
{
   SetConfig();
   ReadConfig();
 #if  0 
   u8 reg256[256];
   u8  i=0;

   for(i=0;i<100;i++)
   {
     memset(reg256, 0,sizeof(reg256));
	 sprintf(reg256,"test save and read num:%d \r\n",i);
	 DF_WP_pageWrite(CycleStart_offset+i,reg256,256);
   }	
   rt_kprintf("\r\n WP DF write over \r\n");

    for(i=0;i<100;i++)
   {
     memset(reg256, 0,sizeof(reg256));
	 DF_WP_pageRead(CycleStart_offset+i,reg256,256);
	 rt_kprintf("read:%s",reg256);
   }	
  #endif	
}





