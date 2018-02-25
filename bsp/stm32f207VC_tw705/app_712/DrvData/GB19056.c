/*
     GB19056.C
*/

#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>
#include <stm32f2xx_usart.h>

#include  <stdlib.h>//数字转换成字符串
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "App_gsm.h"
#include "GB19056.h"

#include "Usbh_conf.h"

#include "ffconf.h"
#include "ff.h"
#include "SPI_SD_driver.h"
#include <finsh.h>
#include "diskio.h"





#define   OUT_PUT        ;//SERIAL_OUTput(type)
#define   USB_OUT       ;// USB_OUTput(CMD,add_fcs_byte)



//    GB  数据导出
#define GB_USB_OUT_idle          0
#define GB_USB_OUT_start         1
#define GB_USB_OUT_running       2
#define GB_USB_OUT_end           3

#define GB_OUT_TYPE_Serial         1
#define GB_OUT_TYPE_USB            2


struct rt_semaphore GB_RX_sem;  //  gb  接收AA 75

rt_device_t   Udisk_device = RT_NULL;

GB_STRKT GB19056;
static FIL 	usb_file;


int  usb_fd = 0;
u8 usb_outbuf[1500];
u16 usb_outbuf_Wr = 0;
u8    Warn_Play_controlBit = 0x16;



void  LoRa_Write_log(u8 *info, u16 inf_len)
{
      u8  Regstr[256];

		WatchDog_Feed();
		DF_TAKE;
        //erase process    4KByte    16 wp_pages
        if(((cycle_write+CycleStart_offset)%16)==0)
         {
           SST25V_SectorErase_4KByte((cycle_write+CycleStart_offset)*WinBond_PageSIZE);
		   DF_delay_ms(50);
		   rt_kprintf("\r\n  Erase Sector:%d",(cycle_write+CycleStart_offset)/16);
         }  

		memset(Regstr,0,sizeof(Regstr));
		memcpy(Regstr,info,inf_len);
		DF_WP_pageWrite(cycle_write+CycleStart_offset,Regstr,256); 
		//---- updata pointer	-------------
		cycle_write++;
		if(cycle_write >LORA_Record_MAX)
		 {
		   cycle_write = 0;
		   cycle_read=1;
		 }  
		DF_Write_RecordAdd(cycle_write, cycle_read, TYPE_CycleAdd);
		DF_delay_us(20);
		//-------------------------------
		DF_RELEASE;

		rt_kprintf("\r\n wrtie: %s",Regstr);        
		
 }


void LoRa_Read_log(u8 Type)
{
  FRESULT file_res;
  u32 len = 0;
  u8  Reg_in[300];
  u32  i=0,counter=1,startpagenum=0,outputMax=0;

  if((cycle_write==0)&&(cycle_read==0))
  {
    rt_kprintf("\r\n no  record save");
    return;
  }	
 
  DF_TAKE;
 
  //  case 0: 
  if(cycle_read==0)
  {
     if(cycle_write>=LORA_LOG_OUT_MAX)
	 	 outputMax=LORA_LOG_OUT_MAX;
	 else
	 	outputMax=cycle_write;
	 
	 	  
     for(i=0;i<outputMax;i++)
     {
        startpagenum=CycleStart_offset+cycle_write-1-i; 

			memset(Reg_in,0,sizeof(Reg_in));
			sprintf(Reg_in,"%00005d   ",counter++);
			DF_WP_pageRead(startpagenum,Reg_in+strlen(Reg_in),256);

       if(Type==0)
       	{
			f_lseek( &usb_file, usb_file.fsize);
			file_res = f_write(&usb_file, Reg_in, strlen(Reg_in),&len);
			if(file_res)
			rt_kprintf("res=%d ", file_res);  
       	}
	   else
	   	{
           rt_kprintf("\r\n ReadA: %s",Reg_in);
	   	}
     }

  }
  else
  if(cycle_read==1)  // 表示存储满过
  {
    // case 1  :
     if(cycle_write==0)
	   {
	     // ------------ 
   		 for(i=0;i<LORA_LOG_OUT_MAX;i++)
		 {  
		    startpagenum=CycleStart_offset+LORA_Record_MAX-i;
			memset(Reg_in,0,sizeof(Reg_in));
			sprintf(Reg_in,"%00005d   ",counter++);
		    DF_WP_pageRead(startpagenum,Reg_in+strlen(Reg_in),256);
             // DF_WP_pageRead(startpagenum,Reg_in,256);
			
				if(Type==0)
		       	{
					f_lseek( &usb_file, usb_file.fsize);
					file_res = f_write(&usb_file, Reg_in, strlen(Reg_in),&len);
					if(file_res)
					rt_kprintf("res=%d ", file_res);  
		       	}
			   else
			   	{
		           rt_kprintf("\r\n ReadB: %s",Reg_in);
			   	} 
		  }	 
		 
	    }	
    //  case 2:
       else
       	{ 
       	         if(cycle_write>=LORA_LOG_OUT_MAX)
				 	 outputMax=LORA_LOG_OUT_MAX;
				 else
				 	outputMax=cycle_write;
				 
			     for(i=0;i<outputMax;i++)
			     {
			        startpagenum=CycleStart_offset+cycle_write-1-i;

					 memset(Reg_in,0,sizeof(Reg_in));
					 sprintf(Reg_in,"%00005d   ",counter++);
				     DF_WP_pageRead(startpagenum,Reg_in+strlen(Reg_in),256);
		             // DF_WP_pageRead(startpagenum,Reg_in,256);


						if(Type==0)
				       	{
							f_lseek( &usb_file, usb_file.fsize);
							file_res = f_write(&usb_file, Reg_in, strlen(Reg_in),&len);
							if(file_res)
							rt_kprintf("res=%d ", file_res);  
				       	}
					   else
					   	{
				           rt_kprintf("\r\n ReadC: %s",Reg_in);
					   	}
			     }
				 
				 if(outputMax<LORA_LOG_OUT_MAX)
				 {
				        outputMax=LORA_LOG_OUT_MAX-outputMax;
                        for(i=0;i<outputMax;i++)
					     {
					        startpagenum=CycleStart_offset+LORA_Record_MAX-i;

							memset(Reg_in,0,sizeof(Reg_in));
							sprintf(Reg_in,"%00005d   ",counter++);
		                    DF_WP_pageRead(startpagenum,Reg_in+strlen(Reg_in),256);
                            //DF_WP_pageRead(startpagenum,Reg_in,256);


								if(Type==0)
						       	{
									f_lseek( &usb_file, usb_file.fsize);
									file_res = f_write(&usb_file, Reg_in, strlen(Reg_in),&len);
									if(file_res)
									rt_kprintf("res=%d ", file_res);  
						       	}
							   else
							   	{
						           rt_kprintf("\r\n ReadD: %s",Reg_in);
							   	}
					     }     				 
				 }
       	   
       			 
       	}	 
	 
 }

    DF_RELEASE;
}
FINSH_FUNCTION_EXPORT(LoRa_Read_log, LoRa_Read_log());


void Lora_WriteLOG(u8* instr)
{
    LoRa_Write_log(instr,strlen(instr));
}
FINSH_FUNCTION_EXPORT(Lora_WriteLOG, Lora_WriteLOG());

void Lora_Tx(u8* txStr)
{  
   u8   TX_str_Log[256]; 
  //u8   left_len=0,current_add=0;
  // u8   tx_32[33]; 
   u16  intxlen=strlen(txStr);
   
   memset(TX_str_Log,0,sizeof(TX_str_Log));
   sprintf((char *)TX_str_Log, "TX: %c%c%c%c%c%c_%c%c%c%c%c%c  info:", (time_now.year / 10 + 0x30), (time_now.year % 10 + 0x30), (time_now.month / 10 + 0x30), (time_now.month % 10 + 0x30), \
				(time_now.day / 10 + 0x30), (time_now.day % 10 + 0x30), (time_now.hour / 10 + 0x30), (time_now.hour % 10 + 0x30), (time_now.min / 10 + 0x30), (time_now.min % 10 + 0x30),\
				 (time_now.sec/ 10 + 0x30), (time_now.sec% 10 + 0x30));
   strcat(TX_str_Log,txStr);		
   //  --- 存储log  -----
   Lora_WriteLOG(TX_str_Log);
   // --- 串口发送原始信息----   
   rt_kprintf("U3Tx:%s",TX_str_Log);  // debug 

   memset(TX_str_Log,0,sizeof(TX_str_Log));
   // --  填写定向发送的地址
   TX_str_Log[0]=0xFF;//(SysConf_struct.LORA_dest1_ADDRESS>>8);
   TX_str_Log[1]=0xFF;//(u8)SysConf_struct.LORA_dest1_ADDRESS;
   TX_str_Log[2]=SysConf_struct.LORA_SYS_Channel;  //   LORA  系统频点
   ;
   memcpy(TX_str_Log+3,txStr,intxlen);

  #if  0
   left_len=intxlen+3;
   current_add=0;

   while(left_len)
   	{
   	   if(left_len>32)	   	  
       {
         memset(tx_32,0,sizeof(tx_32));
		 memcpy(tx_32,TX_str_Log+current_add,32);
         rt_device_write(&Device_UsrSerial, 0, ( const void *)tx_32, (rt_size_t)32);
         current_add+=32;
		 left_len-=32;
		 
   	   	}	 
	   else
	   { 
	      memset(tx_32,0,sizeof(tx_32));
		  memcpy(tx_32,TX_str_Log+current_add,left_len);
	     rt_device_write(&Device_UsrSerial, 0, ( const void *)tx_32, (rt_size_t)left_len);
		 left_len=0;  // send over
	   }
	   rt_kprintf("\r\n info:%s",tx_32); 
   	} 
   #endif
   rt_device_write(&Device_UsrSerial, 0, ( const void *)TX_str_Log, (rt_size_t)intxlen+3);  
   delay_ms(30);
   OutPrint_HEX("Lora 发送原始消息",TX_str_Log,intxlen+3);
}
FINSH_FUNCTION_EXPORT(Lora_Tx, Lora_Tx(str));


void lora_clear(void)
{
  cycle_write=0;
  cycle_read=0;
  DF_Write_RecordAdd(cycle_write, cycle_read, TYPE_CycleAdd);
}
FINSH_FUNCTION_EXPORT(lora_clear, lora_clear());


void lora_dest_addr(u32 address)
{
    rt_kprintf("\r\n lora set Dest address(HEX)=0x%0004X  address(OD)=%d ",address,address);
    SysConf_struct.LORA_dest1_ADDRESS=address;
    Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));

}
FINSH_FUNCTION_EXPORT(lora_dest_addr, lora_dest_addr(u32 address));


void lora_dest_ch(u8 channel) 
{
    rt_kprintf("\r\n lora set Dest channel=%d ",channel);
    SysConf_struct.LORA_dest1_Channel=channel;
    Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
}
FINSH_FUNCTION_EXPORT(lora_dest_ch,lora_dest_ch(u8 channel));


void lora_set_baud(u32 baud)
{
    rt_kprintf("\r\n lora set BaudRate=%d ",baud);
    SysConf_struct.LORA_Baud=baud;
    Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
}
FINSH_FUNCTION_EXPORT(lora_set_baud,lora_set_baud(u32 baud));


void lora_type(u8 type)
{
	rt_kprintf("\r\n lora set  type=  %d   0:  中继   1: 雷达监测点   2:  道口接收点   3: 便携终端 ",type);
	SysConf_struct.LORA_TYPE=type;
    Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
}
FINSH_FUNCTION_EXPORT(lora_type,lora_type(u8 type));



void lora_direction(u8 type)
{       
	SysConf_struct.LORA_DIRECTION=type;
	rt_kprintf("\r\n	   Direction=  %d	0:EE 东   1: SS 南   2: WW 西  3: NN 北  4: ES 东南  5: EN  东北  6: WS  西南 7: WN 西北",SysConf_struct.LORA_DIRECTION);
    Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
}
FINSH_FUNCTION_EXPORT(lora_direction,lora_direction(u8 type));


void lora_set_desc(u8* desc)
{
    rt_kprintf("\r\n lora set desc=%s ",desc);
    memset( SysConf_struct.LORA_PointDesc,0,sizeof( SysConf_struct.LORA_PointDesc));
	memcpy(SysConf_struct.LORA_PointDesc,desc,strlen(desc));
    Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
}
FINSH_FUNCTION_EXPORT(lora_set_desc,lora_set_desc(u8* desc));


void lora_info(void)
{ 
    rt_kprintf("\r\n //----------------------------------------------------");
    rt_kprintf("\r\n lora  Info=   %s ",SysConf_struct.LORA_PointDesc);
	rt_kprintf("\r\n       L_address(HEX)=  0x%0004X  address(OD)=  %d ",SysConf_struct.LORA_Local_ADDRESS,SysConf_struct.LORA_Local_ADDRESS);
    rt_kprintf("\r\n       D_channel(HEX)=  0x%02X    L_channel=  %d ",SysConf_struct.LORA_Local_Channel,SysConf_struct.LORA_Local_Channel);	
	rt_kprintf("\r\n       D_address(HEX)=  0x%0004X  address(OD)=  %d ",SysConf_struct.LORA_dest1_ADDRESS,SysConf_struct.LORA_dest1_ADDRESS);
    rt_kprintf("\r\n       D_channel(HEX)=  0x%02X    D_channel(0D)=  %d ",SysConf_struct.LORA_dest1_Channel,SysConf_struct.LORA_dest1_Channel);
    rt_kprintf("\r\n       System_channel(HEX)=  0x%02X    System_channel(0D)=  %d ",SysConf_struct.LORA_SYS_Channel,SysConf_struct.LORA_SYS_Channel);	


	rt_kprintf("\r\n       baud=  %d ",SysConf_struct.LORA_Baud);
	rt_kprintf("\r\n       type=  %d   0:  中继   1: 雷达监测点   2:  道口接收 3:便携终端 ",SysConf_struct.LORA_TYPE);
                            //    0      1      2      3        4      5     6       7       
                            //   EE    SS   WW    NN   ES    EN    WS   WN
	rt_kprintf("\r\n	   Direction=  %d	0:EE 东   1: SS 南   2: WW 西  3: NN 北  4: ES 东南  5: EN  东北  6: WS  西南 7: WN 西北",SysConf_struct.LORA_DIRECTION);
    rt_kprintf("\r\n       RTC 校准状态:  %d    0: 未校准    1:  已经校准",SysConf_struct.RTC_updated);
	rt_kprintf("\r\n //----------------------------------------------------");
}
FINSH_FUNCTION_EXPORT(lora_info, lora_info());

void lora_sethelp(void)
{
    rt_kprintf("\r\n desc:    %d",SysConf_struct.LORA_PointDesc);
	rt_kprintf("\r\n L_addr:    %d      0x0004X ",SysConf_struct.LORA_Local_ADDRESS,SysConf_struct.LORA_Local_ADDRESS);
    rt_kprintf("\r\n L_channel: %d ",SysConf_struct.LORA_Local_Channel);	
	rt_kprintf("\r\n D_addr:    %d      0x0004X ",SysConf_struct.LORA_dest1_ADDRESS,SysConf_struct.LORA_dest1_ADDRESS);
    rt_kprintf("\r\n D_channel: %d      0x02X",SysConf_struct.LORA_dest1_Channel,SysConf_struct.LORA_dest1_Channel);
	rt_kprintf("\r\n baud:    %d ",SysConf_struct.LORA_Baud);
	rt_kprintf("\r\n type:    %d ",SysConf_struct.LORA_TYPE);
}
FINSH_FUNCTION_EXPORT(lora_sethelp, lora_sethelp());



void USB_OUTput(u8  CMD, u8 fcsAdd)
{
    u32 len = 0;
    FRESULT res;
    if(fcsAdd == 1)
        usb_outbuf[usb_outbuf_Wr++] = GB19056.usb_xor_fcs;
    f_lseek( &usb_file, usb_file.fsize);
    res = f_write(&usb_file, usb_outbuf, usb_outbuf_Wr, &len);
    if(res)
        rt_kprintf("res=%d ", res);

}

u8  BCD2HEX(u8 BCDbyte)
{
    u8  value = 0;

    value = (BCDbyte >> 4) * 10 + (BCDbyte & 0x0F);
    return  value;
}


void  gb_usb_out(void)
{
    /*
               导出信息到USB device
      */
    if(USB_Disk_RunStatus() == USB_FIND)
    {
       rt_thread_delay(5); // delay for protect 
       
   		if(GB19056.usb_exacute_output==0) 	
        {

            GB19056.usb_write_step = GB_USB_OUT_start;
            GB19056.usb_exacute_output = 1;

            if(GB19056.workstate == 0)
                rt_kprintf("\r\n USB 数据导出\r\n");
        }
    }
    else
    {
        GB19056.usb_exacute_output=0;  // output  forbiden 
        //if(GB19056.workstate == 0)
           // rt_kprintf("\r\n 没有检测到 USB device \r\n");
    }

}
FINSH_FUNCTION_EXPORT(gb_usb_out, gb_usb_out());



/*记录仪数据交互状态*/
ALIGN(RT_ALIGN_SIZE)

char gbdrv_thread_stack[2048];
struct rt_thread gbdrv_thread;

void thread_GBData_mode( void *parameter )
{
    u8 str_12len[15];
    u8  i = 0;
	u32 len = 0;
    FRESULT file_res;
	const char* lnw="my name is nathan \r\n";
	u8  dev_info[300];
	/*定义一个函数指针，用作结果处理	*/

    rt_err_t res;

    while( 1 )
    {
        // part 1:  check  USB device  input
         gb_usb_out();
 
        // part2:  USB  output
        if(GB19056.usb_write_step == GB_USB_OUT_start)
        {
            //------ U disk
            Udisk_dev = rt_device_find("udisk");
            if (Udisk_dev != RT_NULL)
            {
                res = rt_device_open(Udisk_dev, RT_DEVICE_OFLAG_RDWR);
                if(res == RT_EOK)
                {
                    GB19056.usb_xor_fcs = 0; // clear fcs
                    //  创建文件
                    memset(GB19056.Usbfilename, 0, sizeof(GB19056.Usbfilename));
                    //  standard  stytle
                    // sprintf((char*)GB19056.Usbfilename,"/udisk/D%c%c%c%c%c%c_%c%c%c%c_%s.VDR",(time_now.year/10+0x30),(time_now.year%10+0x30),(time_now.month/10+0x30),(time_now.month%10+0x30),\
                    // (time_now.day/10+0x30),(time_now.day%10+0x30),(time_now.hour/10+0x30),(time_now.hour%10+0x30),(time_now.min/10+0x30),(time_now.min%10+0x30),Vechicle_Info.Vech_Num);
                    //  debug  add second
                    sprintf((char *)GB19056.Usbfilename, "1:LORA_%c%c%c%c%c%c_%c%c%c%c%c%c_%s.txt", (time_now.year / 10 + 0x30), (time_now.year % 10 + 0x30), (time_now.month / 10 + 0x30), (time_now.month % 10 + 0x30), \
                            (time_now.day / 10 + 0x30), (time_now.day % 10 + 0x30), (time_now.hour / 10 + 0x30), (time_now.hour % 10 + 0x30), (time_now.min / 10 + 0x30), (time_now.min % 10 + 0x30),\
                             (time_now.sec/ 10 + 0x30), (time_now.sec% 10 + 0x30),"Log");

                    //usb_fd=open((const char*)GB19056.Usbfilename, (O_CREAT|O_WRONLY|O_TRUNC), 0 );	  // 创建U 盘文件
                    res = f_open(&usb_file, GB19056.Usbfilename, FA_READ | FA_WRITE | FA_OPEN_ALWAYS );

                    if(GB19056.workstate == 0)
                        rt_kprintf(" \r\n udiskfile: %s  create res=%d	 \r\n", GB19056.Usbfilename, usb_fd);
                    if(usb_fd >= 0)
                    {

                        if(GB19056.workstate == 0)
                            rt_kprintf("\r\n			  创建Drv名称: %s \r\n ", GB19056.Usbfilename);
                        WatchDog_Feed();

                        GB19056.usb_write_step = GB_USB_OUT_running; // goto  next step
                    }
                    else
                    {
                        if(GB19056.workstate == 0)
                            rt_kprintf(" \r\n udiskfile create Fail   \r\n", GB19056.Usbfilename, usb_fd);
                        GB19056.usb_write_step = GB_USB_OUT_idle;
                    }
                }
            }
        }
        if(GB19056.usb_write_step == GB_USB_OUT_running)
        {
          #if 1
           
			//   Device info
		    memset(dev_info,0,sizeof(dev_info));
		    sprintf(dev_info,"DeviceName:  LORA Moudle  \r\n DeviceID:%s \r\n LORA_Local_ADDRESS:%s\r\n",\
				"001001","0x1234");
			f_lseek( &usb_file, usb_file.fsize);
			file_res = f_write(&usb_file, dev_info, strlen(dev_info), &len);
			if(file_res)
			rt_kprintf("res=%d ", file_res);  

		  #endif
		     //  Data  outpu 
		     LoRa_Read_log(0);
				
            GB19056.usb_write_step = GB_USB_OUT_end;
        }
        if(GB19056.usb_write_step == GB_USB_OUT_end)
        {

            // close(usb_fd);
            f_close(&usb_file);
            GB19056.usb_write_step = GB_USB_OUT_idle;
			GB19056.usb_exacute_output=2;             // output  operation over  not  run again 

            if(GB19056.workstate == 0)
                rt_kprintf(" \r\n usboutput   over! \r\n");
            //------------------------------------
            Menu_txt_state = 1;
			LCD_DISP_Clear();
            pMenuItem = &Menu_TXT;
            pMenuItem->show();
            pMenuItem->timetick( 10 );
            pMenuItem->keypress( 10 );
            //--------------------------------------
        }

        //-------------------------------------------
        rt_thread_delay(35);   // RT_TICK_PER_SECOND / 50
    }

}

void GB_Drv_app_init(void)
{
    rt_err_t result;


    rt_sem_init(&GB_RX_sem, "gbRxSem", 0, 0);

    //------------------------------------------------------
    result = rt_thread_init(&gbdrv_thread,
                            "GB_DRV",
                            thread_GBData_mode, RT_NULL,
                            &gbdrv_thread_stack[0], sizeof(gbdrv_thread_stack),
                            Prio_GBDRV, 10);


    if (result == RT_EOK)
    {
        rt_thread_startup(&gbdrv_thread);
    }
}



void  gb_work(u8 value)
{
    if(value)
    {
        GB19056.workstate = 1;
        rt_kprintf(" GB_Data_enable\r\n");
    }
    else
    {
        GB19056.workstate = 0;
        rt_kprintf(" GB_Data_disable\r\n");
    }

}
FINSH_FUNCTION_EXPORT(gb_work, gb_work(1 : 0));

