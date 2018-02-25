/*
     App_808.C
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
#include "usbh_usr.h"


/* 定时器的控制块 */
static rt_timer_t timer_app;


//------- change  on  2013 -7-24  --------
rt_thread_t app_tid = RT_NULL; // app 线程 pid



//----- app_thread   rx     gsm_thread  data  related -----
ALIGN(RT_ALIGN_SIZE)
//static  struct rt_semaphore app_rx_gsmdata_sem;  //  app 提供数据 给gsm发送信号量


// Dataflash  Operate   Mutex







//----- app_thread   rx     gps_thread  data  related -----
//ALIGN(RT_ALIGN_SIZE)
//static  MSG_Q_TYPE  app_rx_gps_infoStruct;  // app   接收从gsm  来的数据结构
//static  struct rt_semaphore app_rx_gps_sem;  //  app 提供数据 给gps发送信号量

//----- app_thread   rx    485 _thread  data  related -----
//ALIGN(RT_ALIGN_SIZE)
//static  MSG_Q_TYPE  app_rx_485_infoStruct;  // app   接收从gsm  来的数据结构
//static  struct rt_semaphore app_rx_485_sem;  //  app 提供数据 给gps发送信号量


u8  TF_test_workState = 0;

rt_device_t   Udisk_dev = RT_NULL;
u8 Udisk_filename[30];
int  udisk_fd = 0;

u32       WarnTimer = 0;

u8   OneSec_CounterApp = 0;
u32  app_thread_runCounter = 0;
u32  gps_thread_runCounter = 0;



//  1. MsgQueue Rx
void  App_thread_timer(void)
{
    app_thread_runCounter++;
    if(app_thread_runCounter > 300)	// 400* app_thread_delay(dur)
    {
        reset();
    }
}

void  gps_thread_timer(void)
{
    gps_thread_runCounter++;
    if((gps_thread_runCounter > 300) ) // 400* app_thread_delay(dur)
    {
        reset();
    }
}

void  App808_tick_counter(void)
{
    Systerm_Reset_counter++;
    if((Systerm_Reset_counter > Max_SystemCounter) )
    {
        Systerm_Reset_counter = 0;
        //rt_kprintf("\r\n Sysem  Control   Reset \r\n");
        reset();
    }

}

void SIMID_Convert_SIMCODE( void )
{

}


static void timeout_app(void   *parameter)
{
    //  100ms  =Dur


    GPRS_GSM_PowerOFF_Working();
    //---------  Step timer
    Dial_step_Single_10ms_timer();

	
    //--------------------------------------
    OneSec_CounterApp++;
    if(OneSec_CounterApp >= 5)
    {
        OneSec_CounterApp = 0;

        //----------------------------------
        if(DataLink_Status())
        {
            ;
        }

        //  RTC 8546  Output Check  ---  
		Get_RTC_enableFlag=1;

        App_thread_timer();

        //  system timer
        App808_tick_counter();

        //-----------------------------------
        if( ReadCycle_status == RdCycle_SdOver)
        {
            ReadCycle_timer++;
            if(ReadCycle_timer > 8) //5s No resulat 
            {
                ReadCycle_timer = 0;
                ReadCycle_status = RdCycle_RdytoSD;
            }
        }
        WatchDog_Feed();

		test_ack_timer();

        Lora_TTS_play_Process();

    }



}



ALIGN(RT_ALIGN_SIZE)
char app808_thread_stack[4096];
struct rt_thread app808_thread;

static void App808_thread_entry(void *parameter)
{

    //    u32  a=0;

    // rt_kprintf("\r\n ---> app808 thread start !\r\n");

    TIM3_Config();
    //  step 3:    usb host init	   	    	//  step  4:   TF card Init
    usbh_init();

    
    Lora_MD_PINS_INIT();
	

    /* watch dog init */
    WatchDogInit();

    //BUZZER
    GPIO_Config_PWM();
    TIM_Config_PWM();
    buzzer_onoff(0);

    while (1)
    {

        rt_thread_delay(23);
        //    485   related  over

		//  LoRa RX 
		 LORA_Rx_Process();
         
	    //  RTC 8546  Output Check  ---
	    if(Get_RTC_enableFlag==1)
	    {	        
            RTC8564_Get();	
            //LORA_RUN.RX_MSG_ID++;  // add for test
			Get_RTC_enableFlag=0; // Clear   
		}
        //----------------------------------------
        app_thread_runCounter = 0;
        //--------------------------------------------------------
    }
}

/* init app808  */
void Protocol_app_init(void)
{
    rt_err_t result;


    //---------  timer_app ----------
    // 5.1. create  timer     100ms=Dur
    timer_app = rt_timer_create("tim_app", timeout_app, RT_NULL, 20, RT_TIMER_FLAG_PERIODIC);
    //  5.2. start timer
    if(timer_app != RT_NULL)
        rt_timer_start(timer_app);

    result = rt_thread_init(&app808_thread,
                            "app808",
                            App808_thread_entry, RT_NULL,
                            &app808_thread_stack[0], sizeof(app808_thread_stack),
                            Prio_App808, 10);

    if (result == RT_EOK)
    {
        rt_thread_startup(&app808_thread);
    }

	Device_UsrSerial_regist();	  //  
}



