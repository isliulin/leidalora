/*
       Device  User Serial 232
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
#include "Device_CAN.h"
#include <finsh.h>
#include "Device_UsrSerial.h"


// ----- LORA related   ------
//  Note  :  当前应用只设置地址和信道号,HEAD 采用C0  (掉电保存)   1A 传输速率  C4:定向发送方式
//                                             HEAD   ADDH  ADDL  Baud  CHNL    TX_MODE
u8	 Lora_config_CMD[6] = {0xC0, 0x12, 0x34, 0x1A, 0x17, 0xC4};
LORA_STATE LORA_RUN;


u8   U3_Rx[256];
u8   U3_content[256];
u16   U3_content_len = 0;
u8   U3_flag = 0;
u16   U3_rxCounter = 0;

// 油耗相关
#define   ABNORMAL_MAXTIMES      30
#define   Zero_Clear_MAXTIMES     60
#define   OIL_CONNECT_MAXTIME      300  //5 分钟






YH   Oil;


#ifdef RT_USING_DEVICE
struct rt_device  Device_UsrSerial;
#endif



unsigned long AssicBufToUL(char * buf,unsigned int num)
{
 unsigned char tempChar;
 unsigned int i,j;
 unsigned long retLong=0;
 
 for(i=0;i<num;i++)
 	{
 	tempChar=(unsigned char)buf[i];
	if((tempChar>='0')&&(tempChar<='9'))
		{
	 	retLong*=10;
		retLong+=tempChar-'0';
		}
	else
		{
		return retLong;
		}
 	}
 return retLong;
}

uint8_t u3_process_YH( uint8_t * pinfo )
{
	//检查数据完整性,执行数据转换
	uint8_t		i;
	uint8_t		buf[32];
	uint8_t		commacount	= 0, count = 0;
	uint8_t		*psrc		= pinfo + 7; //指向开始位置
	uint8_t		crc = 0,crc2;
	u32  temp_u32data=0;

	for(i=0;i<52;i++) 
		{
		crc += pinfo[i];
		}
	//rt_kprintf("\n   CRC1    =0x%2X",crc); 
	while( *psrc++ )
	{
		if(( *psrc != ',' )&&(*psrc != '#'))
		{
			buf[count++]	= *psrc;
			buf[count]		= 0;
			if(count+1 >= sizeof(buf))
				return 1;
			continue;
		}
		commacount++;
		switch( commacount )
		{
			case 1: /*协议相关内容，4字节*/
				break;

			case 2: /*硬件相关版本等，3字节*/
				break;

			case 3: /*设备上线时长，千时，百时，十时，个时，十分，个分；6字节*/
				break;

			case 4: /*平滑处理后的液位值；5字节*/
				if(count<5)
					return 0;
				Oil.oil_average_value = AssicBufToUL(buf,5);
				//rt_kprintf("\n 液位平均值=%d",Oil.oil_average_value);
				break;
			case 5: /*数据滤波等级；1字节*/
				break;
			case 6: /*车辆运行和停止状态；2字节*/
				break;
			case 7: /*当前实时液位正负1cm范围内的液位个数；2字节*/
				break;
			case 8: /*接收信号灵明度，共85级；2字节*/
				break;
			case 9: /*信号强度，最大99；2字节*/
				break;
			case 10: /*实时液位，精度0.1mm；5字节*/
				if(count<5)
					return 0;
				Oil.oil_realtime_value = AssicBufToUL(buf,5);
				//rt_kprintf(" 实时值=%d",Oil.oil_realtime_value);
				break;
			case 11: /*满量程，默认为800；5字节*/
				if(count<5)
					return 0;
				temp_u32data = AssicBufToUL(buf,5);
				//rt_kprintf(" 满量程=%d",temp_u32data);
				break;
			case 12: /*从"*"开始前面52个字符的和；2字节*/
				  Ascii_To_Hex(buf,&crc2,1);  
				//rt_kprintf("\n   CRC2    =0x%2X",crc2);  
				if(crc2 == crc)
				 {	
				   //  Data come  ,  update  every packet
				    Oil.oil_YH_no_data_Counter=0;

                   //  check the   change  
					if(Oil.oil_realtime_value!= Oil.oil_average_value)  // 实时数值和 平均不等更新实时数值
						{
						  Oil.oil_value = Oil.oil_average_value;
						 if((GB19056.workstate == 0)&&(DispContent))
						     rt_kprintf("\r\n Finalvalue update =%d",Oil.oil_value);

						   //  update Oil workstate
						   if(Oil.oil_realtime_value)
				            {
				               Oil.oil_YH_workstate=OIL_NORMAL;
							   Oil.oil_YH_no_data_Counter=0;
							   Oil.oil_YH_0_value_cacheCounter=0;
							   Oil.oil_YH_Abnormal_counter=0;
							   Warn_Status[0] &= ~0x02; //   油量异常还原正常
						   	}  
						   else
						   	{   // check  with vehicle  speed 
						   	  if(Oil.oil_YH_workstate==OIL_NORMAL)
						   	  {
								   	    if(Speed_gps >100)  //  Speed_gps > 10 km/h    
		                                {
		                                   Oil.oil_YH_Abnormal_counter++;
										   if(Oil.oil_YH_Abnormal_counter>ABNORMAL_MAXTIMES)  //  30次是300s   5分钟
										   	{
										   	   Oil.oil_YH_workstate=OIL_ABNORMAL;
											   Oil.oil_YH_Abnormal_counter=0;
											    Warn_Status[0] |= 0x02; //   油量异常还原正常
		                                      // rt_kprintf("\r\n 油耗盒工作异常"); 
											   
										   	}
										    Oil.oil_YH_0_value_cacheCounter=0; 

								   	    } 
										else
										{
										   if(Oil.oil_YH_Abnormal_counter)
										   	{
										   	   Oil.oil_YH_0_value_cacheCounter++;
											   if(Oil.oil_YH_0_value_cacheCounter>Zero_Clear_MAXTIMES)
											   	{
		                                           Oil.oil_YH_0_value_cacheCounter=0;
									               Oil.oil_YH_Abnormal_counter=0;    // just  clear coutner  not  change the state
									              // rt_kprintf("\r\n 速度小于10km 清除中"); 
											   	}
										   	}							    

										}
								
						       }
						   	}
						}
				    // 	Debug related
						if((GB19056.workstate == 0)&&(DispContent))
					       rt_kprintf("\r\n Average =%d,realtime=%d,final=%d 剩余量=%d.%d升  speed=%d  \r\n",Oil.oil_average_value,Oil.oil_realtime_value,Oil.oil_value,Oil.oil_value/10,Oil.oil_value%10,Speed_gps);  
				 } 
				break;
		}
		count	= 0;
		buf[0]	= 0;
	}
	return 9;
}


//   检查油耗盒的连接状态      in     1s  
void Oil_Sensor_Connect_Checking(void)
{
  if(Oil.oil_YH_workstate)
   {
     Oil.oil_YH_no_data_Counter++;
	 if(Oil.oil_YH_no_data_Counter>OIL_CONNECT_MAXTIME)
	 {
	       Oil.oil_YH_no_data_Counter=0;
           Oil.oil_YH_workstate=OIL_NOCONNECT;
	 	   Oil.oil_YH_no_data_Counter=0;
	 	   Oil.oil_YH_0_value_cacheCounter=0;
	 	   Oil.oil_YH_Abnormal_counter=0;    // 油耗异常状态
	 	   Warn_Status[0] &= ~0x02; //   油量异常还原正常
	 	   	if((GB19056.workstate == 0)&&(DispContent))
	 	   		rt_kprintf("\r\n     油耗盒断开"); 
  	 }
	
   }
}



void  Lora_const_style_stuff(void)
{
    //TTS_play("2017年1月11日20时34分22秒,消息挨低,22")
  #if 0
	if(LORA_RUN.HandsendFlag==1)
	{
	    memset(LORA_RUN.TX_buff,0,sizeof(LORA_RUN.TX_buff));
		LORA_RUN.TX_buff[0]=UDP_dataPacket_flag;
		sprintf(LORA_RUN.TX_buff+1," %d-%d-%d %d:%d:%d  CMDID=%d", time_now.year, time_now.month, time_now.day, time_now.hour, \
			  	   time_now.min,time_now.sec,&LORA_RUN.MSG_ID); 
	    
	    Lora_Tx(LORA_RUN.TX_buff); 
		LORA_RUN.HandsendFlag=0;
	}	
  #endif	

    if(SysConf_struct.LORA_TYPE==LORA_RADRCHECK)    //  非雷达点就显示接收内容
	{
		 //  ----  显示部分
		memset(LORA_RUN.Tx_Disp,0,sizeof(LORA_RUN.Tx_Disp));	
		/* 
		sprintf(LORA_RUN.Tx_Disp,"%c20%02d年%02d月%02d日%02d时%02d分%02d秒,消息挨低%d",UDP_dataPacket_flag,time_now.year, time_now.month,time_now.day,\
								  time_now.hour,time_now.min,time_now.sec,LORA_RUN.MSG_ID);
		rt_kprintf("\r\n Rada_disp:%s",LORA_RUN.Tx_Disp);
		*/
		sprintf(LORA_RUN.Tx_Disp,"%c%02d时%02d分%02d秒,消息ID%d",UDP_dataPacket_flag,\
								  time_now.hour,time_now.min,time_now.sec,LORA_RUN.MSG_ID); 
		rt_kprintf("\r\n Rada_disp:%s",LORA_RUN.Tx_Disp);
		LORA_RUN.Tx_Disp[strlen(LORA_RUN.Tx_Disp)]=0x0D;
		LORA_RUN.Tx_Disp[strlen(LORA_RUN.Tx_Disp)]=0x0A;
		//strcat(LORA_RUN.Tx_Disp,"\r\n");
	    Lora_Tx(LORA_RUN.Tx_Disp); 
    }
	//--- 送显示  ----
    //------------------------------------
    Menu_txt_state = 6;
    pMenuItem = &Menu_TXT;
    pMenuItem->show();
    //pMenuItem->timetick( 10 );
   // pMenuItem->keypress( 10 );
    //--------------------------------------

	
}

void LoRA_TX_Process(void)
{
   if(LORA_RUN.SD_Enable==1)
   	{
        Lora_const_style_stuff();
		LORA_RUN.MSG_ID++;
        LORA_RUN.SD_Enable=0;
   	}

}



void LORA_Rx_Process(void)
{
   u8  iRX;
   u8  rx_in[256];
   u8  Lora_TX[256];
   u8  ack_value=0;
   u32 value_get=0;
   u16 log_len=0;

   
     /*
               

         */
	if(U3_flag)
	{
		//if((GB19056.workstate == 0)&&(DispContent)) 
		rt_kprintf("U3Rx:%s", U3_content);
		//u3_process_YH(U3_content,); 
		memset(rx_in,0,sizeof(rx_in));	
		#if 1
		sprintf((char *)rx_in, "RX: %c%c%c%c%c%c_%c%c%c%c%c%c  info:", (time_now.year / 10 + 0x30), (time_now.year % 10 + 0x30), (time_now.month / 10 + 0x30), (time_now.month % 10 + 0x30), \
				(time_now.day / 10 + 0x30), (time_now.day % 10 + 0x30), (time_now.hour / 10 + 0x30), (time_now.hour % 10 + 0x30), (time_now.min / 10 + 0x30), (time_now.min % 10 + 0x30),\
				 (time_now.sec/ 10 + 0x30), (time_now.sec% 10 + 0x30));
		log_len=strlen(rx_in);
		memcpy(rx_in+log_len,U3_content,U3_content_len);  
		//LoRa_Write_log(rx_in,log_len+U3_content_len);      // Lora_WriteLOG(rx_in);
		Lora_WriteLOG(rx_in);
		#endif
		//---------------------------------------
		 // 中继站  (收到信息后，在当前频点上广播发送)
		if(SysConf_struct.LORA_TYPE==LORA_RELAYSTAION)   
		{
		   rt_thread_delay(30+(SysConf_struct.LORA_Local_ADDRESS%10)*10);//地址不同延时不同
           memset(Lora_TX,0,sizeof(Lora_TX));    
		   memcpy(Lora_TX,U3_content,U3_content_len);
		   strcat(Lora_TX,"\n");	
           Lora_Tx(Lora_TX); 
		   memset(LORA_RUN.Tx_Disp,0,sizeof(LORA_RUN.Tx_Disp));	
		   memcpy(LORA_RUN.Tx_Disp,U3_content,U3_content_len-1); // 去掉 0D 0A 
		   LORA_RUN.SD_Enable=1;
		}

        //   道口播报站
        if(SysConf_struct.LORA_TYPE==LORA_ENDPLAY)   
        {
           /* 
		  TDateTime Reg_datetime;

		 
		  sscanf(U3_content+1, "%u.%u.%u.%u.%u.%u.%u", (u32 *)&Reg_datetime.year, (u32 *)&Reg_datetime.month, (u32 *)&Reg_datetime.day, (u32 *)&Reg_datetime.hour, \
		  	   (u32 *)&Reg_datetime.min,(u32 *)&Reg_datetime.sec,(u32 *)&LORA_RUN.MSG_ID);
		  memset(Lora_TX,0,sizeof(Lora_TX));  
		  sprintf(Lora_TX,"%c20%02d年%d月%d日%d时%d分%d秒,消息挨低%d",U3_content[0],time_now.year, time_now.month,time_now.day,\
		  	                        time_now.hour,time_now.min,time_now.sec,LORA_RUN.MSG_ID);
		  rt_kprintf("\r\n TTS_pLay:%s",Lora_TX);
		  */ 
          TTS_play(U3_content);
		  memcpy(rx_in+strlen(rx_in),U3_content,U3_content_len);   
		   Lora_WriteLOG(rx_in); 
		  memset(LORA_RUN.Tx_Disp,0,sizeof(LORA_RUN.Tx_Disp));	
		  memcpy(LORA_RUN.Tx_Disp,U3_content,U3_content_len-1); // 去掉 0D 0A  
		  LORA_RUN.SD_Enable=1; 
        }

        //    雷达监测点
        //  no  code  here   
		
	#if  0	
        if(strncmp(U3_content,"TTS:",4)==0)
        {
           TTS_play(U3_content+4);
		   memcpy(rx_in+strlen(rx_in),U3_content,U3_content_len);  
		   Lora_WriteLOG(rx_in);
		   
        }

		if(strncmp(U3_content,"DAT:",4)==0)
        {
		   memcpy(rx_in+strlen(rx_in),U3_content,U3_content_len);  
		   Lora_WriteLOG(rx_in);
        }

		if(strncmp(U3_content,"SET:",4)==0)   // 设置需要一起处理
		{
           memcpy(rx_in+strlen(rx_in),U3_content,U3_content_len);  
		   Lora_WriteLOG(rx_in);
		   rt_thread_delay(50);
           //----- judge
           if(strncmp(U3_content+4,"addr:",5)==0)
           { 
              value_get=1;
              sscanf(U3_content+9,"%d",(u32 *)&value_get);			  
			  lora_L_addr(value_get);
      
           }
		   if(strncmp(U3_content+4,"channel:",8)==0)
           {
             value_get=1; 
             sscanf(U3_content+12,"%d",(u32 *)&value_get);			  
			 lora_L_ch(value_get);

           }
		   if(strncmp(U3_content+4,"baud:",5)==0)
           {
              value_get=1;
              sscanf(U3_content+9,"%d",(u32 *)&value_get);			  
			  lora_set_baud(value_get);

           }
		  if(strncmp(U3_content+4,"desc:",5)==0)
           {
              value_get=1; 		  
			  lora_set_desc(U3_content+9);

           }	   

           //  ---- stuff  ack info
		   memset(ack_str,0,sizeof(ack_str));
		   if(ack_value==1)
		   	 strcat(ack_str,"\r\n Set Succed!");
		   else
		   	 strcat(ack_str,"\r\n Set Fail!");
		   sprintf(ack_str+strlen(ack_str),"   Piont Info ADDR=%d ,ChannelNum=%d Desc=%s",SysConf_struct.LORA_Local_ADDRESS,SysConf_struct.LORA_Local_Channel,SysConf_struct.LORA_PointDesc);
	       Lora_Tx(ack_str); 
		   
		}
		if(strncmp(U3_content,"READ:",5)==0)
		{
           memcpy(rx_in+strlen(rx_in),U3_content,U3_content_len);  
		   Lora_WriteLOG(rx_in);
		   rt_thread_delay(50);
           
		   sprintf(ack_str,"\r\n ACK Point info   ADDR=%d ,ChannelNum=%d Desc=%s  Baud=%d",SysConf_struct.LORA_Local_ADDRESS,SysConf_struct.LORA_Local_Channel,SysConf_struct.LORA_PointDesc,SysConf_struct.LORA_Baud);
           Lora_Tx(ack_str); 
		}
      #endif
		//--------------------------------------
		U3_content_len=0;
		U3_flag=0;
	}	

}


u16  Protocol_808_Decode_Good(u8 *Instr , u8 *Outstr, u16  in_len) // 解析指定buffer :  UDP_HEX_Rx
{
    //-----------------------------------
    u16 i = 0, decode_len = 0;

    // 1.  clear  write_counter
    decode_len = 0; //clear DecodeLen

    // 2   decode process
    for(i = 0; i < in_len; i++)
    {
        if((Instr[i] == 0x7d) && (Instr[i + 1] == 0x02))
        {
            Outstr[decode_len] = 0x7e;
            i++;
        }
        else if((Instr[i] == 0x7d) && (Instr[i + 1] == 0x01))
        {
            Outstr[decode_len] = 0x7d;
            i++;
        }
        else
        {
            Outstr[decode_len] = Instr[i];
        }
        decode_len++;
    }
    //  3.  The  End
    return decode_len;
}

void u3_RxHandler(unsigned char rx_data)
{
#if 0
    if(U3_flag)
    {
        U3_Rx[U3_rxCounter++] = rx_data;
        if(rx_data == 0x7e)
        {
            U3_content_len = Protocol_808_Decode_Good(U3_Rx, U3_content, U3_rxCounter);

            LORA_Rx_Process();

            U3_flag = 0;
            U3_rxCounter = 0;
        }

    }
    else if((rx_data == 0x7e) && (U3_flag == 0))
    {
        U3_Rx[U3_rxCounter++] = rx_data;
        U3_flag = 1;
    }
    else
        U3_rxCounter = 0;
#endif

    if( rx_data != 0x0A )
    {
        U3_Rx[U3_rxCounter++] = rx_data;
    }
    else
    {
        U3_flag = 1; 
		memset(U3_content,0,sizeof(U3_content));
		memcpy(U3_content,U3_Rx,U3_rxCounter);
		U3_content_len=U3_rxCounter;
		U3_rxCounter=0; 
		//LORA_Rx_Process(); 
    }

}

void Device_UsrSerial_putc(char c)
{
    USART_SendData( USART3, c );
    while( USART_GetFlagStatus( USART3, USART_FLAG_TC ) == RESET )
    {
    }
}




void u3_txdata(u8* txStr, u16 tx_len)
{   
   rt_device_write(&Device_UsrSerial, 0, ( const void *)txStr, (rt_size_t) tx_len);
}
FINSH_FUNCTION_EXPORT(u3_txdata, u3_txdata(str,len));


void lora_mode(u8 value)
{
  rt_kprintf("\r\n mode =%d  1: 电瓶高",value);
  if(value==1)
  	{
        LORA_MD0_HIGH;
        LORA_MD1_HIGH;
  	}
  else
  	{
  	    LORA_MD1_LOW;
		LORA_MD0_LOW;
  	}

}
FINSH_FUNCTION_EXPORT(lora_mode, lora_mode(value));

void lora_send(void)
{
  rt_kprintf("\r\n lora 命令发送消息");
  LORA_RUN.SD_Enable=1;
  LORA_RUN.HandsendFlag=1;
}
FINSH_FUNCTION_EXPORT(lora_send, lora_send());




static rt_err_t   Device_UsrSerial_init( rt_device_t dev )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;


    //  1 . Clock
    RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    /* Enable USART3 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    //   2.  GPIO
    /* Configure USART3 Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure USART3 Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Connect alternate function */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    //  3.  Interrupt
    /* Enable the USART3 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    //   4.  uart  Initial
    USART_InitStructure.USART_BaudRate = 9600;    //CAN2
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(USART3, ENABLE);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);


    return RT_EOK;
}

static rt_err_t Device_UsrSerial_open( rt_device_t dev, rt_uint16_t oflag )
{
    return RT_EOK;
}
static rt_err_t Device_UsrSerial_close( rt_device_t dev )
{
    return RT_EOK;
}

static rt_size_t Device_UsrSerial_read( rt_device_t dev, rt_off_t pos, void *buff, rt_size_t count )
{

    return RT_EOK;
}

static rt_size_t Device_UsrSerial_write( rt_device_t dev, rt_off_t pos, const void *buff, rt_size_t count )
{
    unsigned int  Info_len485 = 0;
    const char		*p	= (const char *)buff;


    Info_len485 = (unsigned int)count;
    /* empty console output */
    //--------  add by  nathanlnw ---------
    while (Info_len485)
    {
        Device_UsrSerial_putc (*p++);
        Info_len485--;
    }
    //--------  add by  nathanlnw  --------
    return RT_EOK;
}
static rt_err_t Device_UsrSerial_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
    return RT_EOK;
}


void  Device_UsrSerial_regist(void )
{
    Device_UsrSerial.type	= RT_Device_Class_Char;
    Device_UsrSerial.init	=   Device_UsrSerial_init;
    Device_UsrSerial.open	=  Device_UsrSerial_open;
    Device_UsrSerial.close	=  Device_UsrSerial_close;
    Device_UsrSerial.read	=  Device_UsrSerial_read;
    Device_UsrSerial.write	=  Device_UsrSerial_write;
    Device_UsrSerial.control = Device_UsrSerial_control;

    rt_device_register( &Device_UsrSerial, "Usr232", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE );
    rt_device_init( &Device_UsrSerial );
}
