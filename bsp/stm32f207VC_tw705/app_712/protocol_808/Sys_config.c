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
#define   LORA_SYS_CHANNEL        23        //17   Ĭ���ŵ�


/*
      ����    0x1236       0x19       25
      �м�  0x1234         0x17       23  

*/
/*
                        0x0000   -----   0x00FF  �������з���
                        0x0100   -----   0x0FFF  ��Ʒ������
                        0x1000   -----   0xF000  Զ��������
                       */



ALIGN(RT_ALIGN_SIZE)
SYS_CONF          SysConf_struct;   //  ϵͳ����
SYS_CONF          SysConf_struct_BAK;   //  ϵͳ����
SYS_CONF          SysConf_struct_BAK2;   //  ϵͳ����


ALIGN(RT_ALIGN_SIZE)





//----------  Basic  Config---------------------------
u8      DeviceNumberID[13];//="800130100001";    // ����DeviceID    ---- �ӱ����ͨ��
u8      SimID_12D[13]; // ����ID  ���� 12 λ ��λ Ϊ 0

u8          RemoteIP_Dnsr[4]={255,255,255,255}; 
u8		RemoteIP_main[4]={111,113,14,154};  //{111,113,14,154}; ���ı���         
u16		RemotePort_main= 9131;//ɽ���к�����   7000
u8		RemoteIP_aux[4]={60,28,50,210};    //{60,28,50,210}
u16		RemotePort_aux=4000; 
//           Link2  Related 
u8      Remote_Link2_IP[4]={111,113,14,154}; 
u16     Remote_Link2_Port=7000;     



u8           APN_String[30] = "UNINET"; //"CMNET";   //  �ӱ����ͨ  �ƶ��Ŀ�
u8           DomainNameStr[50] = "jt1.gghypt.net"; ; // ����  ���ͨup.gps960.com //jt1.gghypt.net
u8           DomainNameStr_aux[50] = "jt2.gghypt.net";   //"www.sina.com";//jt2.gghypt.net
u8          TriggerSDsatus = 0x80; // �����������ϱ�״̬λ


//---------  SytemCounter ------------------
u32  Systerm_Reset_counter = 0;
u8   DistanceWT_Flag = 0; //  д��̱�־λ
u8   SYSTEM_Reset_FLAG = 0;      // ϵͳ��λ��־λ
u32  Device_type = 0x00000001; //Ӳ������   STM32103  ��A1
u32  Firmware_ver = 0x0000001; // ����汾
u8   ISP_resetFlag = 0;      //Զ��������λ��־λ



/*
       ϵͳ������Ϣд��
*/
u8  SysConfig_init(void)
{

    //  1. Stuff
    //   ϵͳ�汾
    SysConf_struct.Version_ID = SYSID;	
	SysConf_struct.LORA_Baud=9600;
    SysConf_struct.LORA_Local_ADDRESS=0x0000;
	SysConf_struct.LORA_Local_Channel=0;
	SysConf_struct.LORA_dest1_ADDRESS=LORA_DEFAULT_ADDR;  
	SysConf_struct.LORA_dest1_Channel=LORA_DEFAULT_CHANNEL;
	SysConf_struct.LORA_SYS_Channel=LORA_SYS_CHANNEL;  //	LORA  ϵͳƵ��
	SysConf_struct.LORA_TYPE=LORA_HANDLE_DEV;  // �ն� 
	SysConf_struct.LORA_DIRECTION=0;
	SysConf_struct.RTC_updated=0;    //  RTC У׼״̬��־λ
	memset( SysConf_struct.LORA_PointDesc,0,sizeof( SysConf_struct.LORA_PointDesc));
	memcpy(SysConf_struct.LORA_PointDesc,"�ն˹�������",12);
    

    cycle_write=0;
	cycle_read=0;
    DF_Write_RecordAdd(cycle_write, cycle_read, TYPE_CycleAdd);
	
    //    2. Operate
    return(Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct)));

}

void SysConfig_Read(void)
{
   u16   res[3];
    //��ȡϵͳ������Ϣ
   

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
    if(res[0] && res[1] && res[2])	 // ȫ������
    {
        rt_kprintf("\r\n SysConf_structȫ��ʧ��! \r\n");
        rt_kprintf("\r\n need all recover");
        SysConfig_init();// д��ϵͳ������Ϣ
        reset(); 
    }
    else if(res[0] && res[1])	 //    seg1  seg2  ������˵��  BAK error
    {
        // org  bak2 ---ok 	 bak---error
        if((u8)(SysConf_struct.Version_ID>> 8) != 0xFF) // �ж���ȷ���ǲ��� FF
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
    else if(res[0] && res[2])	//	seg1  seg3	  ������˵�� BAK2  error
    {
        // org	bak  ---ok		 bak2 -----error
        if((u8)(SysConf_struct.Version_ID >> 8) != 0xFF) // �ж���ȷ���ǲ��� FF
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
    else if(res[1] && res[2])	//	seg2  seg3	  ������˵�� org  error
    {
        //	bak  bak2 --ok	   org---error
        if((u8)(SysConf_struct.Version_ID >> 8) != 0xFF) // �ж���ȷ���ǲ��� FF
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
        rt_kprintf("\r\n SysConf_struct ��ȡУ��ɹ�! \r\n"); 

    //--------------------------------------------------------------------------------------------------


   //  �������
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
    // 1.  ��ȡconfig ����      0 :�ɹ�    1 :  ʧ��
    Api_Config_read(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
    //rt_kprintf("\r\nRead Save SYSID\r\n");
    //  2. ��ȡ�ɹ�  ���ж�  �汾ID
    if(SysConf_struct.Version_ID != SYSID) //SYSID)   //  check  wether need  update  or not
    {
        rt_kprintf("\r\n ID not Equal   Saved==0x%X ,  Read==0x%X !\r\n", SYSID, SysConf_struct.Version_ID);
        SysConf_struct.Version_ID = SYSID; // update  ID

		 SysConfig_init();   //  д��ϵͳ������Ϣ 
    }
    else
        rt_kprintf("\r\n Config Already Exist!\r\n");
}

void ReadConfig(void)
{
    u16   res[3];

    DF_delay_ms(500);
    
    SysConfig_Read();  //��ȡϵͳ������Ϣ 

	
	rt_kprintf("\r\n  cyc_recordMax=%d    cyc_outMax=%d",LORA_Record_MAX,LORA_LOG_OUT_MAX);
    rt_kprintf("\r\n  cycwrite=%d     cycread=%d  \r\n", cycle_write,cycle_read);
	lora_info();
}

/*
�����������ļ�
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





