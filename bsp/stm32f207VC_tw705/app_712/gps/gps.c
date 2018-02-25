/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		// 文件名
 * Author:			// 作者
 * Date:			// 日期
 * Description:		// 模块描述
 * Version:			// 版本信息
 * Function List:	// 主要函数及其功能
 *     1. -------
 * History:			// 历史修改记录
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/
#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "stm32f2xx.h"
#include "stm32f2xx_i2c.h"
#include "gps.h"
#include "App_moduleConfig.h"

#include <finsh.h>


void RTC8564_Set(RTCTIME time);


RTCTIME rtc_current;
u8       Reg_common[8];
u8  rtc_tmp[10];
u8   Get_RTC_enableFlag=0;


unsigned bcd2bin(unsigned char val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

unsigned char bin2bcd(unsigned val)
{
	return ((val / 10) << 4) + val % 10;
}


void RTC_8564_IIC_INIT(void)
{
      GPIO_InitTypeDef        gpio_init;
	  I2C_InitTypeDef i2c_config;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
 

	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource7, GPIO_AF_I2C1);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE); 

    gpio_init.GPIO_Mode = GPIO_Mode_AF;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_OType = GPIO_OType_OD;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    gpio_init.GPIO_Pin	 = GPIO_Pin_6|GPIO_Pin_7;	 
    GPIO_Init(GPIOB, &gpio_init);

	
    //------------------- PB5-----------------------------
    gpio_init.GPIO_Pin	 = RTC_8546_INT_PIN;			
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_Init(RTC_8546_INT_PORT, &gpio_init);


    i2c_config.I2C_ClockSpeed = 400000;
    i2c_config.I2C_Mode = I2C_Mode_I2C;
    i2c_config.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c_config.I2C_OwnAddress1=0;        
    i2c_config.I2C_Ack = I2C_Ack_Enable;
    i2c_config.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    I2C_Init(I2C1,&i2c_config);
    I2C_Cmd(I2C1,ENABLE);



}


unsigned char I2C_ReadOneByte(unsigned char addr)
{
       unsigned char res=0;
       uint32_t I2C_Timeout=I2C_TIMEOUT;
   
	   while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) // Added by Najoua	27/08/2008	
	   	{ 
	         if((I2C_Timeout--) == 0)
	               return 0;
	   	}
	   
       I2C_GenerateSTART(I2C1, ENABLE);      
       while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
      
       I2C_Send7bitAddress(I2C1,RTC_WR,I2C_Direction_Transmitter);
       while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
 
      
       I2C_SendData(I2C1,addr);
       while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED));
 
       I2C_GenerateSTART(I2C1,ENABLE);
       while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
      
       I2C_Send7bitAddress(I2C1,RTC_WR,I2C_Direction_Receiver);
       while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
      
       while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));
       
       res=I2C1->DR;
       
        I2C_AcknowledgeConfig(I2C1, DISABLE);//must
       
       I2C_GenerateSTOP(I2C1,ENABLE);
       return res;      
      
}


unsigned char I2C_Read(unsigned char addr,unsigned char *buf,unsigned char num)
{
       while(num--)
       {           
              *buf++=I2C_ReadOneByte(addr++);
                //delay(100);  
       }           
      
       return 0;     
      
}

 
 static unsigned char I2C_WriteOneByte(unsigned char addr,unsigned char value)
 {	  
		 uint32_t I2C_Timeout=I2C_TIMEOUT;
   
	   while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) // Added by Najoua	27/08/2008	
	   	{ 
	         if((I2C_Timeout--) == 0)
	               return 0;
	   	}
	   
		I2C_GenerateSTART(I2C1, ENABLE);	  
		while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
	   
		I2C_Send7bitAddress(I2C1,RTC_WR,I2C_Direction_Transmitter);
		while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  
		I2C_SendData(I2C1,addr);
		while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	   
		I2C_SendData(I2C1,value);
		while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	   
		I2C_GenerateSTOP(I2C1,ENABLE);
		return 0;
 }
  
 unsigned char I2C_Write(unsigned char addr,unsigned char *buf,unsigned char num)
 {
		unsigned char err=0;
	   
		while(num--)
		{
			   if(I2C_WriteOneByte(addr++,*buf++))
			   {
					  err++;
			   }
		}
		if(err)
			   return 1;
		else
			   return 0; 
 }
  


//================================
void I2C_Wrte_Data( u8 WriteAddr, u8* pBuffer,u8 NumByteToWrite)
{	 
   uint32_t I2C_Timeout=I2C_TIMEOUT;
   
   while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) // Added by Najoua	27/08/2008	
   	{ 
         if((I2C_Timeout--) == 0)
               return;
   	}
   I2C_GenerateSTART(I2C1, ENABLE);  
   
  /* Test on EV5 and clear it */   
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));    
  
  /* Send EEPROM address for write */   
  I2C_Send7bitAddress(I2C1, RTC_WR, I2C_Direction_Transmitter) ;  
  
  /* Test on EV6 and clear it */  
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));  
  
  /* Send the EEPROM's internal address to write to */      
  I2C_SendData(I2C1, WriteAddr);     
  /* Test on EV8 and clear it */   
  while(! I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));   
  /* While there is data to be written */  
  while(NumByteToWrite--)     
  	{    
  	    /* Send the current byte */   
		I2C_SendData(I2C1, *pBuffer);    
		/* Point to the next byte to be written */ 
		pBuffer++;  
		/* Test on EV8 and clear it */   
		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));    
		}    
    /* Send STOP condition */   
  I2C_GenerateSTOP(I2C1, ENABLE); 

}


void I2C_Read_Data(u8 ReadAddr,u8* pBuffer,  u16 NumByteToRead)    
{	 
   uint32_t I2C_Timeout=I2C_TIMEOUT;
//*((u8 *)0x4001080c) |=0x80;		
while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) // Added by Najoua  27/08/2008	 
	{ 
         if((I2C_Timeout--) == 0)
               return;
   	}


/* Send START condition */	 
I2C_GenerateSTART(I2C1, ENABLE);

//*((u8 *)0x4001080c) &=~0x80;	
/* Test on EV5 and clear it */	 
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));	 

/* Send EEPROM address for write */  
I2C_Send7bitAddress(I2C1, RTC_WR, I2C_Direction_Transmitter) ;  

 /* Test on EV6 and clear it */  
 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));   
 
 /* Clear EV6 by setting again the PE bit */  
 I2C_Cmd(I2C1, ENABLE);	  
 
 /* Send the EEPROM's internal address to write to */  
 I2C_SendData(I2C1, ReadAddr);		
 
 /* Test on EV8 and clear it */	
 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	  
 
	 /* Send STRAT condition a second time */ 	 
	 I2C_GenerateSTART(I2C1, ENABLE);   
	 /* Test on EV5 and clear it */	 
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));   
	 /* Send EEPROM address for read */	 
	 I2C_Send7bitAddress(I2C1, RTC_WR, I2C_Direction_Receiver);
	 /* Test on EV6 and clear it */   
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) );	
	 /* While there is data to be read */	
	 while(NumByteToRead)		
	 	{ 
	 	if(NumByteToRead == 1)	
			{ 
			/* Disable Acknowledgement */   
			I2C_AcknowledgeConfig(I2C1, DISABLE);	 
			/* Send STOP Condition */	
			I2C_GenerateSTOP(I2C1, ENABLE);	
			} 
		/* Test on EV7 and clear it */	 
		if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))	
			{			
			/* Read a byte from the EEPROM */   
			*pBuffer = I2C_ReceiveData(I2C1);	
			/* Point to the next location where the byte read will be saved	*/	 
			pBuffer++;	  
			/* Decrement the read bytes counter */	
			NumByteToRead--; 		  
			}	  
	  }	 
	 /* Enable Acknowledgement to be ready for another reception */	 
	 I2C_AcknowledgeConfig(I2C1, ENABLE);    
 }  


//  ======= RTC   App  ===================

void RTC8564_Init(void)
{
	delay_ms(10);
	rt_kprintf("\r\n---      RTC INIT start");
	I2C_WriteOneByte(reg_ctr1,0x00); 
	//禁止中断
	I2C_WriteOneByte(reg_ctr2,0x00);	

	if(SysConf_struct.RTC_updated==0)
	{
		//设置时间2017/3/3 10:50:30 5
		rtc_current.year = 0x17;
		rtc_current.month = 0x03;
		rtc_current.day = 0x03;
		rtc_current.hour = 0x10;
		rtc_current.min = 0x11;
		rtc_current.sec=0x12;
		
		RTC8564_Set(rtc_current);
	}	
 
     
	//禁止所有的闹钟
	I2C_WriteOneByte(reg_alarm_min,0x80);	
	I2C_WriteOneByte(reg_alarm_hour,0x80);	
	I2C_WriteOneByte(reg_alarm_day,0x80);	
	I2C_WriteOneByte(reg_alarm_week,0x80);	
	//禁止时钟引脚输出
	I2C_WriteOneByte(reg_clk,0x7F);
	//禁止定时中断
	I2C_WriteOneByte(reg_timer_ctr,0x7F);

	I2C_WriteOneByte(reg_timer,0x01); 


	
}


void RTC8564_Set(RTCTIME time)
{
  #ifdef RTC_8564
	uint8_t getdata=0;
	I2C_Read_Data(reg_sec,&getdata,1);
	if((getdata&0x80)==0x80)  // VL bit
	{
		;//return;//电压信息低
		
	}
	I2C_Read_Data(reg_ctr1,&getdata,1);
	getdata = getdata | 0x20;//0xDF;
	I2C_WriteOneByte(reg_ctr1,getdata);
	//测试
	I2C_Read_Data(reg_ctr1,&getdata,1);
	rt_kprintf("\r\n   Set  STOP BIT    =%02X  ",getdata);
	
	//开始执行时间配置
	if(time.sec<=0x59)
	{
		I2C_WriteOneByte(reg_sec,time.sec);	
		//测试
	I2C_Read_Data(reg_sec,&getdata,1);
		rt_kprintf("\r\n Sec=%02X  ",getdata);
	}
	else
	     return;
	if(time.min<=0x59)
	{
		I2C_WriteOneByte(reg_min,time.min);	
		//测试
	I2C_Read_Data(reg_min,&getdata,1);
		rt_kprintf(" MIN=%02X  ",getdata);
	}
	else
         return;
	if(time.hour<=0x23)
	{
		I2C_WriteOneByte(reg_hour,time.hour);	
		//测试
	I2C_Read_Data(reg_hour,&getdata,1);
		rt_kprintf(" Hour=%02X  ",getdata);
	}
	else
	     return;
	if(time.year<=0x99)
	{
		I2C_WriteOneByte(reg_year,time.year);
		//测试
	I2C_Read_Data(reg_year,&getdata,1);
		rt_kprintf(" Year=%02X  ",getdata);
	}
	else
	     return;
	if(time.month<=12)
	{
		I2C_WriteOneByte(reg_month,time.month);
		//测试
	I2C_Read_Data(reg_month,&getdata,1);
		rt_kprintf(" MONTH=%02X  ",getdata);
	}
	else
	     return;
	if(time.day<=0x31)
	{
		I2C_WriteOneByte(reg_day,time.day);	
		//测试
	I2C_Read_Data(reg_day,&getdata,1);
		rt_kprintf(" Day=%02X  \r\n",getdata);
	}
	else
	    return;
	//SEGGER_RTT_printf(0,"RTC SET\n");
	I2C_Read_Data(reg_ctr1,&getdata,1);
	//rt_kprintf("\r\n Read  old  REG_CTRL1=%02X  ",getdata);
	getdata = 0x00;
	I2C_WriteOneByte(reg_ctr1,getdata);
	//测试
	I2C_Read_Data(reg_ctr1,&getdata,1); 
	//rt_kprintf("\r\n Read  NEW  REG_CTRL1=%02X  ",getdata);

	#endif
}

void RTC8564_Get(void)
{

#ifdef RTC_8564
   u8  getdata=0;
   
	 memset(rtc_tmp,0xEE,8);  
	 I2C_Read_Data(reg_year,&rtc_tmp[0],1);
	 I2C_Read_Data(reg_month,&rtc_tmp[1],1);
	 I2C_Read_Data(reg_day,&rtc_tmp[2],1);
	 I2C_Read_Data(reg_hour,&rtc_tmp[3],1);
	 I2C_Read_Data(reg_min,&rtc_tmp[4],1);
	 I2C_Read_Data(reg_sec,&rtc_tmp[5],1);
	 I2C_Read_Data(reg_week,&rtc_tmp[6],1); 
	
	 if(rtc_tmp[5]&0x80)
	  {
          RTC_8564_IIC_INIT(); 
		  RTC8564_Init();
		  
		  rt_kprintf("\r\n	RTC  ERROR	---Initial again  2 \r\n");  
		  I2C_Read_Data(reg_ctr1,&getdata,1);
		  if(getdata==0x20)   // stop bit enable
		  {
				//printf("\r\n RTC REG_CTRL1=%02X  ",getdata);
				getdata = 0x00;
				I2C_WriteOneByte(reg_ctr1,getdata);
				I2C_Read_Data(reg_ctr1,&getdata,1);
		  }	 
         #if 1  
          if(SysConf_struct.RTC_updated==1)
          {
               SysConf_struct.RTC_updated=0;               
			   Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
               rt_kprintf("\r\n  RTC 电压低  --  RTC 时间不准 \r\n");
		  }
         #endif
		  
	  }	
	 else
	 {
	  Reg_common[0]=rtc_tmp[0];  //  get useful bits  year
	  Reg_common[1]=rtc_tmp[1]&0x1F;  // month	 
	  Reg_common[2]=rtc_tmp[2]&0x3F; // day
	  Reg_common[3]=rtc_tmp[3]&0x3F;  //  get useful bits	 hour
	  Reg_common[4]=rtc_tmp[4]&0x7F; // min 
	  Reg_common[5]=rtc_tmp[5]&0x7F; // sec
	
	  //---sec check ---
	  if((bcd2bin(Reg_common[5])>59)||(bcd2bin(Reg_common[4])>59)||(bcd2bin(Reg_common[3])>23)|| \
		 (bcd2bin(Reg_common[2])>31)||(bcd2bin(Reg_common[1])>12)||(bcd2bin(Reg_common[0])>99)||\
		 ((bcd2bin(Reg_common[0])==0)&&(bcd2bin(Reg_common[1])==0)&&(bcd2bin(Reg_common[2])==0)))
	  {    
		 //  printf("\r\n 获取到的RTC 不合法 需要重新设置 %02X-%02X-%02X %02X:%02X:%02X\r\n",Reg_common[0],Reg_common[1],Reg_common[2],Reg_common[3],Reg_common[4],Reg_common[5]);
		  RTC_8564_IIC_INIT(); 
		  RTC8564_Init();
		   rt_kprintf("\r\n  RTC  ERROR  ---Initial again  1 \r\n");  
		  I2C_Read_Data(reg_ctr1,&getdata,1);
		  if(getdata==0x20)   // stop bit enable
		  {
				//printf("\r\n RTC REG_CTRL1=%02X  ",getdata);
				getdata = 0x00;
				I2C_WriteOneByte(reg_ctr1,getdata);
				I2C_Read_Data(reg_ctr1,&getdata,1);
			//	printf("\r\n RTC  Reset REG_CTRL1=%02X  ",getdata);
		  }		
	  }
	  else	  
	  { 
		memset(rtc_current.BCD_6_Bytes,0,6);
		memcpy(rtc_current.BCD_6_Bytes,Reg_common,6);
		
		//rt_kprintf("\r\n RTC One Bye one   %02X-%02X-%02X %02X:%02X:%02X\r\n",rtc_current.BCD_6_Bytes[0],rtc_current.BCD_6_Bytes[1],rtc_current.BCD_6_Bytes[2],rtc_current.BCD_6_Bytes[3],rtc_current.BCD_6_Bytes[4],rtc_current.BCD_6_Bytes[5]);
	  }  
	 }
	#endif 

}





/************************************** The End Of File **************************************/

