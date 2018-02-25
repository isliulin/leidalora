/*
     HMI  :  Process  LCD  、Printer 、 KeyChecking
*/

#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//数字转换成字符串
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "spi_sd.h"
#include "Usbh_conf.h"





/* HMI  thread */
ALIGN(RT_ALIGN_SIZE)
char HMI_thread_stack[2048]; // 4096
struct rt_thread HMI_thread;

static void HMI_thread_entry(void *parameter)
{
    u8 counter_printer = 0;
    u8 i = 0;



    //--------自检过后操作DF  读取-----
    delay_ms(500);
    DF_init();
    SysConfiguration();    // system config
    
	//------ RTC 8546  chip   Init
    RTC_8564_IIC_INIT();
    RTC8564_Init();
		
    DF_initOver = 1;
   // Gsm_RegisterInit(); 	//	init register states	,then  it  will  power on  the module
   // SIMID_Convert_SIMCODE(); //   translate
      loramodule_get();  
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

	     		
		//_485_process();
	    	
		if(Menu_Number!=3)
			{ 
                  LORA_RUN.SD_waitACK_Flag=0;
		          LORA_RUN.ACK_index=0; 
			}
		else
		if(Menu_Number!=2)
		{
              LORA_RUN.TX_MSG_ID=0;// clear
              _485_speed=0;
		}
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



