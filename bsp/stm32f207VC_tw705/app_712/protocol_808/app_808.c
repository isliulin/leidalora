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
    if((gps_thread_runCounter > 300) && (BD_ISP.ISP_running == 0)) // 400* app_thread_delay(dur)
    {
        reset();
    }
}

void  App808_tick_counter(void)
{
    Systerm_Reset_counter++;
    if((Systerm_Reset_counter > Max_SystemCounter) && (Spd_Using <= 10))
    {
        Systerm_Reset_counter = 0;
        //rt_kprintf("\r\n Sysem  Control   Reset \r\n");
        reset();
    }

}

void SIMID_Convert_SIMCODE( void )
{
    SIM_code[0] = SimID_12D[0] - 0X30;
    SIM_code[0] <<= 4;
    SIM_code[0] |= SimID_12D[1] - 0X30;

    SIM_code[1] = SimID_12D[2] - 0X30;
    SIM_code[1] <<= 4;
    SIM_code[1] |= SimID_12D[3] - 0X30;

    SIM_code[2] = SimID_12D[4] - 0X30;
    SIM_code[2] <<= 4;
    SIM_code[2] |= SimID_12D[5] - 0X30;

    SIM_code[3] = SimID_12D[6] - 0X30;
    SIM_code[3] <<= 4;
    SIM_code[3] |= SimID_12D[7] - 0X30;

    SIM_code[4] = SimID_12D[8] - 0X30;
    SIM_code[4] <<= 4;
    SIM_code[4] |= SimID_12D[9] - 0X30;

    SIM_code[5] = SimID_12D[10] - 0X30;
    SIM_code[5] <<= 4;
    SIM_code[5] |= SimID_12D[11] - 0X30;
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

        App_thread_timer();
        gps_thread_timer();
        GPS_Keep_V_timer();

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


        //--------  多媒体时间信息上传 后处理(不判应答)-----
        //--------------  多媒体上传相关   天地通有时不给多媒体信息上传应答  --------------
        if(MediaObj.Media_transmittingFlag == 1) // clear
        {
            MediaObj.Media_transmittingFlag = 2;
            if(Duomeiti_sdFlag == 1)
            {
                Duomeiti_sdFlag = 0;
                //Video_send_end();
            }
            // rt_kprintf("\r\n  多媒体信息前的多媒体发送完毕 \r\n");
        }
        WatchDog_Feed();

        //   from 485
       

        //  油耗盒连接状态检测
        //Oil_Sensor_Connect_Checking();


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
    Init_ADC();
    gps_io_init();


    CAN_App_Init();	// CAN初始化
    AD_PowerInit();

    /* watch dog init */
    WatchDogInit();

    //BUZZER
    GPIO_Config_PWM();
    TIM_Config_PWM();
    buzzer_onoff(0);

    while (1)
    {

        //   1.   处理相关接收到的   808 数据
        if(Receive_DataFlag == 1)
        {
            memcpy( UDP_HEX_Rx, GSM_HEX, GSM_HEX_len);
            UDP_hexRx_len = GSM_HEX_len;
            TCP_RX_Process(LinkNum);
            Receive_DataFlag = 0;
        }
        // 2.  485  Related  ---------
        //--------------------- 拍照数据处理-----
        if(_485_CameraData_Enable)
        {
            delay_ms(5);
            _485_CameraData_Enable = 0;
        }
        rt_thread_delay(10);
        // 3.    检查顺序存储 gps  标准信息的状态
        app_thread_runCounter = 0;
        // 4.    808   Send data
        if(DataLink_Status() && (CallState == CallState_Idle))
        {
            Do_SendGPSReport_GPRS();
        }
        // 5. ---------------  顺序存储 GPS  -------------------
        if(GPS_getfirst)	 //------必须搜索到经纬度
        {
            if(Current_UDP_sd == 0)
                Save_GPS();
        }
      
        //  10  . Redial_reset_save
        if(Redial_reset_save)
        {
            delay_ms(10);
            delay_ms(8);
            Redial_reset_save = 0;
        }

        //-------     485  TX ------------------------
        //Send_const485(CameraState.TX_485const_Enable);
        rt_thread_delay(13);
        //    485   related  over

		//  LoRa RX 
		  LORA_Rx_Process();

        //   Lora_TX_process
        // LoRA_TX_Process(); 
	    	
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
}



