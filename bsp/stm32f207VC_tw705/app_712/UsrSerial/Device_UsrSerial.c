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
//                                            HEAD   ADDH  ADDL  Baud  CHNL    TX_MODE
u8	 Lora_config_CMD[6] = {0xC0, 0x12, 0x34, 0x1A, 0x17, 0xC4};
u8   Lora_rd_config[3]={0xC1,0xC1,0xC1};
u8   Lora_configging=0;  // 模块配置中 
LORA_STATE LORA_RUN;


u8   U3_Rx[256];
u8   U3_content[256];
u16   U3_content_len = 0;
u8   U3_flag = 0;
u16   U3_rxCounter = 0;








#ifdef RT_USING_DEVICE
struct rt_device  Device_UsrSerial;
#endif



//------------------------------
void test_ack_timer(void)
{
  if(LORA_RUN.test_flag==1)
  	{
        LORA_RUN.test_counter++;
		{
		    if(LORA_RUN.test_counter>4)
             {
                LORA_RUN.test_counter=0;
                LORA_RUN.test_send_flag=1; 
				LORA_RUN.test_flag=0;
		     }
		}

  	}
}

void lora_testack_stuff(void)
{
    u8  TX_CHECK[33];

   memset(TX_CHECK,0,sizeof(TX_CHECK)); 
   memcpy(TX_CHECK,"P1-ACK\r\n",8);    
   Lora_Tx(TX_CHECK);  

}







//---------------------------------------------------------

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



void  Lora_TTS_play_Process(void)
{

    if((LORA_RUN.Played_times)&&(TTS_Var.NeedtoPlay==0)&&(TTS_Var.Playing==0))
    {  
        rt_kprintf("\r\n 重复播放消息 ID=%d   第 %d 次重播 \r\n",LORA_RUN.RX_MSG_ID,3-LORA_RUN.Played_times);
        TTS_play(LORA_RUN.Play_str);	
        LORA_RUN.Played_times--;

		     //  更新显示信息
       if(Menu_Number==1)
		 {		   
	        pMenuItem = &Menu_1_Idle;
	        pMenuItem->show();
			memset(LORA_RUN.ComeDirectStr,0,sizeof(LORA_RUN.ComeDirectStr));
		    memcpy(LORA_RUN.ComeDirectStr,LORA_RUN.Play_str+7,12);
			rt_kprintf("\r\n dispcontent:%s",LORA_RUN.ComeDirectStr);
			LORA_RUN.Come_state=1;
       	}
    }
} 

void  Lora_tts_play_newcome(void)
{
   LORA_RUN.Played_times=2;  // 准备播放3次
}

void Lora_inspect_stuff(void)    //  填充便携台巡检报文
{
   u8  TX_CHECK[33];

   memset(TX_CHECK,0,sizeof(TX_CHECK)); 
   memcpy(TX_CHECK,"CHECK\r\n",7);   
   Lora_Tx(TX_CHECK); 
   
}

void  Lora_const_style_stuff(void)
{
   u8  TX_33BUF[33];

    if(SysConf_struct.LORA_TYPE==LORA_HANDLE_DEV)    //  非雷达点就显示接收内容
	{
		memset(TX_33BUF,0,sizeof(TX_33BUF));	
		memcpy(TX_33BUF,"VA-RS-",6);    // 测试 RS taken
		sprintf(TX_33BUF+6,"%02X-%02X-%02X-%d-%d",\
								  rtc_current.BCD_6_Bytes[3],rtc_current.BCD_6_Bytes[4],rtc_current.BCD_6_Bytes[5],_485_speed,LORA_RUN.TX_MSG_ID); 

       /*      发送消息格式                 
			 EE-HH-MM-SS-SPD-MSGID
			  1  2	3  4  5  6
             */
		rt_kprintf("\r\n Rada_disp:%s",TX_33BUF);
		TX_33BUF[strlen(TX_33BUF)]=0x0D;
		TX_33BUF[strlen(TX_33BUF)]=0x0A;
		//strcat(LORA_RUN.Tx_Disp,"\r\n");
	    Lora_Tx(TX_33BUF); 
    }
	
}

void LoRA_TX_Process(void)
{
   if(LORA_RUN.SD_Enable==1)
   	{
		LORA_RUN.TX_MSG_ID++;		
        Lora_const_style_stuff(); 
        LORA_RUN.SD_Enable=0;
		rt_thread_delay(3);
   	}
   else
    if(LORA_RUN.SD_check_Enable==1)
   	{
       Lora_inspect_stuff();
       LORA_RUN.SD_check_Enable=0;
	   LORA_RUN.SD_waitACK_Flag=1;    //发送了等待应答 
	   rt_thread_delay(3);
   	}
	else            //  准格尔测试  -----------
   	if(LORA_RUN.test_send_flag==1)
   	{
   	   lora_testack_stuff();
       LORA_RUN.test_send_flag=0;
	   rt_thread_delay(3);
   	}

}


u8  lora_tts_decode(u8* orgstr)
{   
    u8   inlen=strlen(orgstr),write_len=0;
	u8   hour_in,min_in,second_in,spd_in;
	u32  msgid_in=0;
       /*      发送消息格式                 
			 EE-HH-MM-SS-SPD-MSGID
			  1  2	3  4  5  6
             */
    if(inlen<15)     
		return true;
   if((orgstr[2]=='-')&&(orgstr[5]=='-')&&(orgstr[8]=='-')&&(orgstr[11]=='-'))
   	{
       
	
	   sscanf(orgstr+6, "%u-%u-%u-%u-%u", (u32 *)&hour_in, (u32 *)&min_in, (u32 *)&second_in, (u32 *)&spd_in,(u32 *)&msgid_in);
	  // rt_kprintf("\r\n Printout:    %d+%d+%d=%d=%d \r\n",hour_in,min_in,second_in,spd_in,msgid_in);

        //  判断来源消息ID 的情况 
       if(LORA_RUN.RX_MSG_ID==msgid_in)
       	{
            rt_kprintf("\r\n  收到重复ID的消息, 重复ID=%d \r\n",msgid_in);
			 return true;
       	}
	   else
         {
             LORA_RUN.RX_MSG_ID=msgid_in;

	   	 }	 
	   
	  
        memset(LORA_RUN.Play_str,0,sizeof(LORA_RUN.Play_str));
        
		strcat(LORA_RUN.Play_str,"请注意:"); 
        write_len=strlen(LORA_RUN.Play_str);

       
        	
		#if  1
        //   RS  RX     S1   X1   ES  EX   
         if(orgstr[3]=='S')
            strcat(LORA_RUN.Play_str+write_len,"上行有机车通过,");      
		 else
		 if(orgstr[3]=='X')	
		 	strcat(LORA_RUN.Play_str+write_len,"下行有机车通过,");
         else 
		 if(orgstr[3]=='R')	
         {
         	   if(orgstr[4]=='S')
                   strcat(LORA_RUN.Play_str+write_len,"上行有机车通过,");      
		       else
		       if(orgstr[4]=='X')	
		 	       strcat(LORA_RUN.Play_str+write_len,"下行有机车通过,");
         }
		 else
		  if(orgstr[3]=='E')	
         {
         	   if(orgstr[4]=='S')
                   strcat(LORA_RUN.Play_str+write_len,"上行有机车通过,");      
		       else
		       if(orgstr[5]=='X')	
		 	       strcat(LORA_RUN.Play_str+write_len,"下行有机车通过,");
         }	
 
        write_len=strlen(LORA_RUN.Play_str); 
		sprintf(LORA_RUN.Play_str+write_len,"车速%d公里每小时,请及时避让,消息序号%d",spd_in,msgid_in);


        //  更新显示信息
         if(Menu_Number!=1)
		 {
		   LCD_DISP_Clear();
		   rt_kprintf("\r\n 其他界面下有消息来了");
         }    
		
        pMenuItem = &Menu_1_Idle;
        pMenuItem->show();
		memset(LORA_RUN.ComeDirectStr,0,sizeof(LORA_RUN.ComeDirectStr));
		memcpy(LORA_RUN.ComeDirectStr,LORA_RUN.Play_str+7,12);
		rt_kprintf("\r\n dispcontent:%s",LORA_RUN.ComeDirectStr);
		LORA_RUN.Come_state=1;

		#endif

		
		//sprintf(LORA_RUN.Play_str)+write_len,"%车速%d公里,消息序号%d",spd_in,msgid_in);

        
		
        LORA_RUN.ComeSPD=spd_in;
		LORA_RUN.RX_MSG_ID=msgid_in;




		

         //   -----  授时----
        rtc_current.year = 0x17;
		rtc_current.month = 0x03;
		rtc_current.day = 0x03;
		rtc_current.hour =bin2bcd(hour_in);
		rtc_current.min = bin2bcd(min_in);
		rtc_current.sec= bin2bcd(second_in);		
		RTC8564_Set(rtc_current);
        //----  update  RTC  ---
          if(SysConf_struct.RTC_updated==0)
          {
               SysConf_struct.RTC_updated=1;               
			   Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
		  }

		//rt_kprintf("播报信息:%s\r\n",Play_str);
	    Lora_tts_play_newcome();	
		LORA_RUN.test_flag=1;    // 准格尔回复应答 
		TTS_play(LORA_RUN.Play_str);	
		 
       return false;
   	}
   else
     return true;
}
FINSH_FUNCTION_EXPORT(lora_tts_decode, lora_tts_decode());



void LORA_Rx_Process(void)
{
   u8  iRX,channel=0;
   u8  rx_in[256];
   u8  Lora_TX[256];
   u8  ack_value=0;
   u32 value_get=0;
   u16 log_len=0,Addr=0;

 channel=0+19;
 channel=10;
   
	if((U3_flag)&&(Lora_configging==0))
	{
		rt_kprintf("\r\nU3Rx:%s", U3_content);

		
		memset(rx_in,0,sizeof(rx_in));	
		#if 1
		sprintf((char *)rx_in, "RX: %c%c%c%c%c%c_%c%c%c%c%c%c  info:", (time_now.year / 10 + 0x30), (time_now.year % 10 + 0x30), (time_now.month / 10 + 0x30), (time_now.month % 10 + 0x30), \
				(time_now.day / 10 + 0x30), (time_now.day % 10 + 0x30), (time_now.hour / 10 + 0x30), (time_now.hour % 10 + 0x30), (time_now.min / 10 + 0x30), (time_now.min % 10 + 0x30),\
				 (time_now.sec/ 10 + 0x30), (time_now.sec% 10 + 0x30));
		log_len=strlen(rx_in);
		memcpy(rx_in+log_len,U3_content,U3_content_len);  
		Lora_WriteLOG(rx_in); 
		//LoRa_Write_log(rx_in,log_len+U3_content_len);      // Lora_WriteLOG(rx_in);
		//Lora_WriteLOG(rx_in);
		#endif
		//---------------------------------------

		 //  巡检 接收判断   XX-ACK
		if(strncmp(U3_content+2,"-ACK",3)==0)  //  
		{
           rt_kprintf("\r\n 便携终端收到巡检应答  from:%c%c \r\n",U3_content[0],U3_content[1]); 			
		   
           if(LORA_RUN.ACK_index<=4)
           {   
                buzzer_onoff(1);
                memset(Lora_TX,0,sizeof(Lora_TX));
				sprintf(Lora_TX, "%d.",LORA_RUN.ACK_index+1);
               //---------------------------------------------------------------
               /*
                             Part2：   发送节点名称   RS  RX                      S1               X1                     ES                  EX     
                                                                                 雷达       上行中继     下行中继   上行道口   下行道口  
                           */
               switch(U3_content[0])
               	{
                   case 'R':
				   	          if(U3_content[1]=='S')                                    
							      memcpy(Lora_TX+2,"上行雷达",8);
							  else
							  if(U3_content[1]=='X')	
                                  memcpy(Lora_TX+2,"下行雷达",8);
					        break;
							
				   case 'S':
				   	          memcpy(Lora_TX+2,"上行中继",8);
							  sprintf(Lora_TX+10,"%d",HexValue(U3_content[1]));
				   	        break;
							
				   case 'X':
				   	          memcpy(Lora_TX+2,"下行中继",8);
				   	          sprintf(Lora_TX+10,"%d",HexValue(U3_content[1]));
				   	        break;
							
							
				   case 'E':    
				   	           if(U3_content[1]=='S')
                                  memcpy(Lora_TX+2,"上行道口",8);
							  else
							  if(U3_content[1]=='X')	
                                  memcpy(Lora_TX+2,"下行道口",8);
							  
				   	        break;
               	}

			    //memset(LORA_RUN.ACK_INFO[LORA_RUN.ACK_index],0,sizeof(LORA_RUN.ACK_INFO));
				memcpy(LORA_RUN.ACK_INFO[LORA_RUN.ACK_index],Lora_TX,strlen(Lora_TX));

				rt_kprintf("\r\n 巡检反馈: %s",LORA_RUN.ACK_INFO[LORA_RUN.ACK_index]); 				
			    LORA_RUN.ACK_index++;
                delay_ms(200); 
				buzzer_onoff(0);
           } 
		  
		}
        else
        //   道口播报站或便携终端
        if(SysConf_struct.LORA_TYPE==LORA_HANDLE_DEV)   
        {
   
		  
		  // 根据格式选择播放信息方式
		  if(lora_tts_decode(U3_content))
             rt_kprintf("\r\n 新消息还没播完");//TTS_play(U3_content);
		  
		 // memcpy(rx_in+strlen(rx_in),U3_content,U3_content_len);   
		  // Lora_WriteLOG(rx_in); 
		  memset(LORA_RUN.Tx_Disp,0,sizeof(LORA_RUN.Tx_Disp));	
		  memcpy(LORA_RUN.Tx_Disp,U3_content,U3_content_len-1); // 去掉 0D 0A  
		 // LORA_RUN.SD_Enable=1; 
        }
		
		//--------------------------------------
		U3_content_len=0;
		U3_flag=0;
	}	
	
	else
	if(Lora_configging==1)
	{
		if(U3_flag)
		{
			  OutPrint_HEX("\r\n===>>>U3Rx:",U3_content,U3_content_len);
			  if( U3_content[0]==0x4F)
				   rt_kprintf("\r\n  LORA set ACK");
		
			  if( U3_content[0]==0xC0)
			  {	
				  rt_kprintf("\r\n  LORA Read ACK OK!");
			
				  rt_kprintf("\r\n Setinfo	confirm  Addr=0x%02X%02X airSPD=0x%02X channel(hex)=0x%02X  (DEC)=%d \r\n",U3_content[1],\
					   U3_content[2],U3_content[3],U3_content[4],U3_content[4]);
 
                   //---------------------------------------------  
	                Addr=((u16)U3_content[1]<<8)+U3_content[2];
					channel=U3_content[4];
                   //---------------------------------------------
                   if((SysConf_struct.LORA_Local_ADDRESS!=Addr)||(SysConf_struct.LORA_Local_Channel!=channel))
				    {
						 SysConf_struct.LORA_Local_ADDRESS=Addr;  
						 SysConf_struct.LORA_Local_Channel=channel;
						 SysConf_struct.LORA_SYS_Channel=channel;
						 Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
						 rt_kprintf("\r\n 设置修改本机信道参数 \r\n");
				    }
                   else
				   if(channel!=SysConf_struct.LORA_SYS_Channel)
				   	{
                        SysConf_struct.LORA_SYS_Channel=channel;
						 Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
						 rt_kprintf("\r\n 开机自检修改本机信道参数 \r\n");
				   	}
				   
				    memcpy(Lora_config_CMD,U3_content,6);// 更新当前RAM
				  
			  }
			  
			  	delay_ms(100);
				LORA_MD0_LOW;
				LORA_MD1_LOW;  
				Lora_configging=0;
				buzzer_onoff(0);
				//--------------------------------------
				U3_content_len=0;
				U3_flag=0;
		} 
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




void LORA_RxHandler(unsigned char rx_data)
{

		if(Lora_configging)
		{
		       U3_Rx[U3_rxCounter++]=rx_data;
       
					 if(U3_rxCounter>=6)
					 {
						 if((U3_Rx[5]==0xC4)&&(U3_Rx[0]==0xC0))
						 {	 
							 U3_content_len =6;
							 memset(U3_content,0,sizeof(U3_content));
							 memcpy(U3_content,U3_Rx,U3_rxCounter); 				   
							 U3_flag = 1;
							 U3_rxCounter = 0;
						 }
						 else
						 {
						   U3_rxCounter=0;
						   U3_flag=0;
						 }
					 }
					 else
					 if(U3_rxCounter==4)
					 {
						 if((U3_Rx[0]==0x4F)&&(U3_Rx[1]==0x4B)&&(U3_Rx[2]==0x0D)&&(U3_Rx[3]==0x0A))
						 {
							U3_content_len =4;
							 memset(U3_content,0,sizeof(U3_content));
							 memcpy(U3_content,U3_Rx,U3_rxCounter); 				   
							 U3_flag = 1;
							 U3_rxCounter = 0;
	 
						 }
	 
					 }	
                     if((U3_Rx[0]!=0x4F)&&(U3_Rx[0]!=0xC0))
                     	{
                     	   U3_flag = 0;
						   U3_rxCounter = 0;

                     	}					 
						 
		  }
	 else
     {
			
		    if( rx_data != 0x0A )
		    {
		        U3_Rx[U3_rxCounter++] = rx_data;
				if(U3_rxCounter>=29)  //  addr  2 byte  ch 1byte  29 byte max
					{
					    U3_rxCounter=0; // clear
					    U3_content_len=0;
				        U3_flag=0;               
					}
		    }
		    else
		    {
		      if(U3_rxCounter<4)   // 长度不合法
		      	{
		      	  U3_rxCounter=0;  // clear
		          U3_content_len=0;
				  U3_flag=0;
		      	}
			   else
		       {
			        U3_flag = 1; 
					memset(U3_content,0,sizeof(U3_content));
					memcpy(U3_content,U3_Rx,U3_rxCounter);
					U3_content_len=U3_rxCounter;
					U3_rxCounter=0; 
		      	}
		    }
	
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
void Lora_channel(u8  channel)
{
   
   rt_kprintf("\r\n System Channel =0x%02X    <=>  %d  ",channel,channel);
   SysConf_struct.LORA_SYS_Channel=channel;
   SysConf_struct.LORA_dest1_Channel=channel;
   Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct)); 

}
FINSH_FUNCTION_EXPORT(Lora_channel,Lora_channel(u8 type));


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

void lora_send(u32 ID,u8  SPD)
{
   rt_kprintf("\r\n 便携台  lora 命令发送消息");
   LORA_RUN.SD_Enable=1;
  
   _485_speed=SPD;
   	LORA_RUN.TX_MSG_ID=ID;  
}
FINSH_FUNCTION_EXPORT(lora_send, lora_send(ID,SPD));

void lora_sd_check(void)
{
   rt_kprintf("\r\n 便携台  发送巡检消息");
   LORA_RUN.SD_check_Enable=1;
}
FINSH_FUNCTION_EXPORT(lora_sd_check, lora_sd_check());


void  loramodule_set(u16 Addr,u8 channel)
{

  
    Lora_config_CMD[1]=(Addr>>8);
	Lora_config_CMD[2]=(u8)Addr;
	Lora_config_CMD[4]=(u8)channel;
  
    LORA_MD0_HIGH;
	LORA_MD1_HIGH;
	delay_ms(200);
	Lora_configging=1;
	buzzer_onoff(1);

    if((SysConf_struct.LORA_Local_ADDRESS!=Addr)||(SysConf_struct.LORA_Local_Channel!=channel))
    {
		 SysConf_struct.LORA_Local_ADDRESS=Addr;  
		 SysConf_struct.LORA_Local_Channel=channel;
		 Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
		 rt_kprintf("\r\n 设置修改本机信道参数 \r\n");
    }	
    //---发送配置信息---    
	rt_device_write(&Device_UsrSerial, 0, ( const void *)Lora_config_CMD,6);   
	OutPrint_HEX("Lora 发送LORA配置信息",Lora_config_CMD,6);
}
FINSH_FUNCTION_EXPORT(loramodule_set, loramodule_set());

void  loramodule_spd(u8 invalue )
{
  
  u32  icounter=160000;
  u16  ack_add=0;

  switch(invalue)
  	{
        case 1:    // 0.3K
                Lora_config_CMD[3]=0x18;
		 	    break;
		case 2:   //  1.2 k
		        Lora_config_CMD[3]=0x19;
			    break;
		case 3:    //  2.4K
		        Lora_config_CMD[3]=0x1A;
			    break;
		default:
                rt_kprintf("\r\n 输入参数有误");
			    return;
  


  	} 
  
  
  LORA_MD0_HIGH;
  LORA_MD1_HIGH;
  delay_ms(200);
  Lora_configging=1;
  buzzer_onoff(1);

  #if 0
  if((SysConf_struct.LORA_Local_ADDRESS!=Addr)||(SysConf_struct.LORA_Local_Channel!=channel))
  {
	   SysConf_struct.LORA_Local_ADDRESS=Addr;	
	   SysConf_struct.LORA_Local_Channel=channel;
	   Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
	   rt_kprintf("\r\n 设置修改本机传输速率参数 \r\n");
  }  
  #endif
  
  //---发送配置信息---	  
  rt_device_write(&Device_UsrSerial, 0, ( const void *)Lora_config_CMD,6);	 
  OutPrint_HEX("Lora 发送LORA配置信息",Lora_config_CMD,6);

}
FINSH_FUNCTION_EXPORT(loramodule_spd, loramodule_spd());


void  loramodule_get(void)
{
    u32  icounter=160000;
	u16  ack_add=0;
	
  
  
    LORA_MD0_HIGH;
	LORA_MD1_HIGH;
	delay_ms(200);
	Lora_configging=1;
	buzzer_onoff(1);
    ////---发送配置信息---    
	rt_device_write(&Device_UsrSerial, 0, ( const void *)Lora_rd_config,3);   
	OutPrint_HEX("Lora 读取LORA配置信息",Lora_rd_config,3);
}
FINSH_FUNCTION_EXPORT(loramodule_get, loramodule_get());

void rtc_set(u8 *instr)
{
  
  u8   year_in,month_in,day_in,spd_in,hour_in,min_in,second_in;

   buzzer_onoff(1);
   sscanf(instr, "%u-%u-%u %u:%u:%u", (u32 *)&year_in, (u32 *)&month_in, (u32 *)&day_in,(u32 *)&hour_in, (u32 *)&min_in, (u32 *)&second_in);
     //   -----  授时----
        rtc_current.year = bin2bcd(year_in);
		rtc_current.month =bin2bcd(month_in);
		rtc_current.day = bin2bcd(day_in);
		rtc_current.hour =bin2bcd(hour_in);
		rtc_current.min = bin2bcd(min_in);
		rtc_current.sec= bin2bcd(second_in);		
		RTC8564_Set(rtc_current);

		  if(SysConf_struct.RTC_updated==0)
          {
               SysConf_struct.RTC_updated=1;               
			   Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
		  }
   buzzer_onoff(0);
}
FINSH_FUNCTION_EXPORT(rtc_set, rtc_set(YY-MM-DD HH-MM-SS));

void rtc_get(u8 value)
{
  rt_kprintf("\r\n Current RTC   %02X-%02X-%02X %02X:%02X:%02X    valuable=%d    \r\n",rtc_current.BCD_6_Bytes[0],rtc_current.BCD_6_Bytes[1],rtc_current.BCD_6_Bytes[2],rtc_current.BCD_6_Bytes[3],rtc_current.BCD_6_Bytes[4],rtc_current.BCD_6_Bytes[5],SysConf_struct.RTC_updated);
  if(value)
  	SysConf_struct.RTC_updated=0;
  else
  	SysConf_struct.RTC_updated=1;
}
FINSH_FUNCTION_EXPORT(rtc_get, rtc_get);


void  Lora_MD_PINS_INIT(void)   
{
    GPIO_InitTypeDef        gpio_init;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    gpio_init.GPIO_Mode = GPIO_Mode_AF;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    //	OUT
    //----------- LORA    MD0    PB8------------------
    gpio_init.GPIO_Pin	= LORA_MD0_PIN;   //------LORA MD0
    gpio_init.GPIO_Mode	= GPIO_Mode_OUT;
    GPIO_Init(LORA_MD0_PORT, &gpio_init);
    
    
    // ------------LORA  MD1   PE7 ---------------
    gpio_init.GPIO_Pin	= LORA_MD1_PIN;   //------LORA MD1
    gpio_init.GPIO_Mode	= GPIO_Mode_OUT;
    GPIO_Init(LORA_MD1_PORT, &gpio_init);

    // ----  正常使用情况下 ----
	LORA_MD0_LOW;
	LORA_MD1_LOW;
}



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
    USART_Cmd(USART3,ENABLE);
	
    USART_ITConfig(USART3,USART_IT_RXNE, ENABLE);
    delay_ms(8);
    memset(LORA_RUN.Tx_Disp,0x20,sizeof(LORA_RUN.Tx_Disp));  //空格
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
