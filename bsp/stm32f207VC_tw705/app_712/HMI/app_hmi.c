/*
     HMI  :  Process  LCD  ��Printer �� KeyChecking
*/

#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//����ת�����ַ���
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "spi_sd.h"
#include "Usbh_conf.h"


unsigned char dayin_data_time[35] = {"\r\n��ӡʱ��:                    \r\n"};
unsigned char dayin_chepaihaoma[26] = {"\r\n���������ƺ���:00000000"};
unsigned char dayin_chepaifenlei[25] = {"\r\n���������Ʒ���:000000"};
unsigned char dayin_cheliangVIN[32] = {"\r\n����VIN:00000000000000000"};
//unsigned char dayin_driver_NUM[40]={"��ʻԱ����:000000000000000000000"};
unsigned char dayin_driver_NUM[40] = {"\r\n��ʻԱ����:000000"};
unsigned char dayin_driver_card[50] = {"\r\n��������ʻ֤����:\r\n   000000000000000000"};
static struct rt_messagequeue	HMI_MsgQue;

void Dayin_Fun(u8 dayin_par)
{
}



/* HMI  thread */
ALIGN(RT_ALIGN_SIZE)
char HMI_thread_stack[2048]; // 4096
struct rt_thread HMI_thread;

static void HMI_thread_entry(void *parameter)
{
    u8 counter_printer = 0;
    u8 i = 0;



    //--------�Լ�������DF  ��ȡ-----
    delay_ms(500);
    DF_init();
    SysConfiguration();    // system config
    DF_initOver = 1;
   // Gsm_RegisterInit(); 	//	init register states	,then  it  will  power on  the module
   // SIMID_Convert_SIMCODE(); //   translate

#ifdef  TFCARD
    TF_Init();
#endif


    //-----------------------------------------------------------------


        pMenuItem = &Menu_1_Idle;
        pMenuItem->show();
        pMenuItem->timetick(10);
        pMenuItem->keypress(10);
	
    while (1)
    {
        pMenuItem->timetick( 10 );
        pMenuItem->keypress(10);

		
		//   Lora_TX_process
         LoRA_TX_Process();   		
        //--------------------------------------------
        rt_thread_delay(25);     //25
    }
}



/* init HMI  */
void HMI_app_init(void)
{
    rt_err_t result;


    result = rt_thread_init(&HMI_thread, "HMI",
                            HMI_thread_entry, RT_NULL,
                            &HMI_thread_stack[0], sizeof(HMI_thread_stack),
                            Prio_HMI, 6);

    if (result == RT_EOK)
    {
        rt_thread_startup(&HMI_thread);
    }
}



