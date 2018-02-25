#include  <stdlib.h>//êy×?×a??3é×?·?′?
#include  <stdio.h>
#include  <string.h>
#include <./App_moduleConfig.h>

unsigned char tickcount = 0;
u8  idle_coutner=0;

static unsigned char Battery[] = {0x00, 0xFC, 0xFF, 0xFF, 0xFC, 0x00}; //8*6

//电池 是否校验特征系数的标志
DECL_BMP(8, 6, Battery);



void GPSGPRS_Status(void)
{

}
void Disp_Idle(void)    // test use
{
     u32 counter=0;
   u8 i=0,j=0,spd=0;
   u8 dispstr[40];

    
    Menu_Number=1; 

	spd=LORA_RUN.ComeSPD; 
	counter=LORA_RUN.RX_MSG_ID;
	
	if((spd >= 100) && (spd < 999))
	{
		Dis_speDer[1] = spd / 100 + '0';
		Dis_speDer[2] = (spd % 100) / 10 + '0';
		Dis_speDer[3] = spd % 10 + '0';
	
	
	}
	else if((spd >= 10) && (spd < 100))
	{
		Dis_speDer[1] = ' ';
		Dis_speDer[2] = (spd / 10) + '0';
		Dis_speDer[3] = spd % 10 + '0';
	}
	else if(spd < 10)
	{
		Dis_speDer[1] = ' ';
		Dis_speDer[2] = ' ';
		Dis_speDer[3] = spd % 10 + '0';
	}
   
  // LCD_DISP_CHINESE_TXT(0, 2,"天津七一二",5);
   LCD_DISP_8x16_TXT(0, 4,"LORA", 4);    
   LCD_DISP_CHINESE_TXT(0, 4,"便携终端", 8);   
  // LCD_DISP_8x16_TXT(30, 0,Dis_speDer, 18);
    // 方向
   if((strlen(LORA_RUN.ComeDirectStr))&&(LORA_RUN.Come_state==1)) 
            LCD_DISP_CHINESE_TXT(1, 2,LORA_RUN.ComeDirectStr, 12);  
   else 
   	        LCD_DISP_8x16_TXT(16, 0,"                    ", 20);   
   
   if(LORA_RUN.Come_state==1)
   	{
	   idle_coutner++;
	   if(idle_coutner>12)
	   	{
	   	  LORA_RUN.Come_state=0;
		  idle_coutner=0;
	   	}  
   	}
   //  速度
   LCD_DISP_8x16_TXT(34, 6,Dis_speDer, 18);

   // RXID
   memset(dispstr,0,sizeof(dispstr));	 
   sprintf(dispstr,"CH:%d MSGID:%d   ",Lora_config_CMD[4],counter);
   LCD_DISP_8x8_TXT(55,1,dispstr,strlen(dispstr));   

   

   if(SysConf_struct.RTC_updated==0)
   	{ 
      LCD_DISP_8x8_TXT(73,0,"Time not calibrated ",20);  
   	}
   else
   	{
       memset(dispstr,0,sizeof(dispstr));    
	   sprintf(dispstr,"      %02X:%02X:%02X      ",rtc_current.BCD_6_Bytes[3],rtc_current.BCD_6_Bytes[4],rtc_current.BCD_6_Bytes[5]);
       LCD_DISP_8x8_TXT(73,0,dispstr,20);  
   }


}

#if 0
void  Disp_Idle(void)
{
  
   u32 counter=0;
   u8 i=0,j=0,spd=0;
   u8 dispstr[40];

	spd=LORA_RUN.ComeSPD;
	counter=LORA_RUN.RX_MSG_ID;
	
	if((spd >= 100) && (spd < 999))
	{
		Dis_speDer[1] = spd / 100 + '0';
		Dis_speDer[2] = (spd % 100) / 10 + '0';
		Dis_speDer[3] = spd % 10 + '0';
	
	
	}
	else if((spd >= 10) && (spd < 100))
	{
		Dis_speDer[1] = ' ';
		Dis_speDer[2] = (spd / 10) + '0';
		Dis_speDer[3] = spd % 10 + '0';
	}
	else if(spd < 10)
	{
		Dis_speDer[1] = ' ';
		Dis_speDer[2] = ' ';
		Dis_speDer[3] = spd % 10 + '0';
	}

  // LCD_DISP_CHINESE_TXT(0, 2,"天津七一二",5);
   LCD_DISP_8x16_TXT(0, 4,"LORA", 4);    
   LCD_DISP_CHINESE_TXT(0, 4,"便携终端", 8);   
  // LCD_DISP_8x16_TXT(30, 0,Dis_speDer, 18);
    // 方向
   if(strlen(LORA_RUN.ComeDirectStr))
            LCD_DISP_CHINESE_TXT(1, 2,LORA_RUN.ComeDirectStr, 12); 
   //  速度
   LCD_DISP_8x16_TXT(34, 6,Dis_speDer, 18);

   // RXID
   memset(dispstr,0,sizeof(dispstr));	 
   sprintf(dispstr,"     MSGID:%06d   ",counter);
   LCD_DISP_8x8_TXT(55,0,dispstr,20);  

   

   if(SysConf_struct.RTC_updated==0)
   	{ 
      LCD_DISP_8x8_TXT(73,0,"Time not calibrated ",20);  
   	}
   else
   	{
       memset(dispstr,0,sizeof(dispstr));    
	   sprintf(dispstr,"      %02X:%02X:%02X      ",rtc_current.BCD_6_Bytes[3],rtc_current.BCD_6_Bytes[4],rtc_current.BCD_6_Bytes[5]);
       LCD_DISP_8x8_TXT(73,0,dispstr,20);  
   }
 #if 0
    u8 i = 0;
    u16  disp_spd = 0;
    u8  Date[3], Time[3];


        Date[0] = time_now.year;
        Date[1] = time_now.month;
        Date[2] = time_now.day;

        Time[0] = time_now.hour;
        Time[1] = time_now.min;
        Time[2] = time_now.sec;
		
    for(i = 0; i < 3; i++)
        Dis_date[2 + i * 3] = Date[i] / 10 + '0';
    for(i = 0; i < 3; i++)
        Dis_date[3 + i * 3] = Date[i] % 10 + '0';

    for(i = 0; i < 3; i++)
        Dis_date[12 + i * 3] = Time[i] / 10 + '0';
    for(i = 0; i < 3; i++)
        Dis_date[13 + i * 3] = Time[i] % 10 + '0';

    //----------------速度--------------------------


#if  0
    //  记录仪认证 用脉冲速度
    // Dis_speDer[0]='C';
    Dis_speDer[0] = ' ';
    disp_spd = Speed_cacu / 10;
#endif
    //--------------------------------------------------------------------
    // if((disp_spd>=100)&&(disp_spd<200))
   if(SysConf_struct.LORA_TYPE==LORA_RADRCHECK)   
   {
	    disp_spd=_485_speed;
	    if((disp_spd >= 100) && (disp_spd < 999))
	    {
	        Dis_speDer[1] = disp_spd / 100 + '0';
	        Dis_speDer[2] = (disp_spd % 100) / 10 + '0';
	        Dis_speDer[3] = disp_spd % 10 + '0';


	    }
	    else if((disp_spd >= 10) && (disp_spd < 100))
	    {
	        Dis_speDer[1] = ' ';
	        Dis_speDer[2] = (disp_spd / 10) + '0';
	        Dis_speDer[3] = disp_spd % 10 + '0';
	    }
	    else if(disp_spd < 10)
	    {
	        Dis_speDer[1] = ' ';
	        Dis_speDer[2] = ' ';
	        Dis_speDer[3] = disp_spd % 10 + '0';
	    }
   	}
    //---------------方向-----------------------------
    memset(Dis_speDer + 10, ' ', 10); // 初始化为空格



    //--------------------------------------------------
   // LCD_DISP_Clear();
	

   // lcd_text12(0, 12, (char *)Dis_date, 20, 1);   
       //LCD_DISP_8x8_TXT(0, 10, (char*)LORA_RUN.Tx_Disp, 18);   
       LCD_DISP_8x8_TXT(0, 20, (char *)Dis_speDer, 18);


     // const  disp  
     LCD_DISP_8x16_TXT(0, 32, "TCB712",6);   
	 LCD_DISP_CHINESE_TXT(0, 52,"天津通广移动部", 14);   

   #endif

 //----  Disp DEMO -----  
   //LCD_DISP_8x16_TXT(0, 16, "TCB712wxdhsyb+TCB712wxdhsyb",27);  //---OK
   //LCD_DISP_8x8_TXT(42,16,"LNWNATHAN@712.cn+LNWNATHAN@712.cn+LNWNATHAN@712.cn",50); //OK
   //LCD_DISP_CHINESE_TXT(0, 2,"天津通广移动部", 14);    
   //LCD_DISP_CHINESE_TXT(2, 4,"天津七一二通信股份有限公司天津七一二通信股份有限公司", 52);    

 #if     0   // 汉字显示  
   LCD_DISP_CHINESE_Char(0x00,0,0xC7E7);
  // delay_ms(3);
   LCD_DISP_CHINESE_Char(0x01,0,0xD0E8);
   //delay_ms(3);
   LCD_DISP_CHINESE_Char(0x02,0x00,0xC7E9);
   // delay_ms(3);
   LCD_DISP_CHINESE_Char(0x03,0x01,0xCDA8);
   // delay_ms(3);
   LCD_DISP_CHINESE_Char(0x04,0x01,0xB9E3);

   LCD_DISP_CHINESE_Char(0x00,0x02,0xCDA8);
   // delay_ms(3);
   LCD_DISP_CHINESE_Char(0x00,0x03,0xB9E3); 
   LCD_DISP_CHINESE_Char(0x00,0x04,0xB9E3); 


   LCD_DISP_CHINESE_Char(0x08,0x00,0xC7E9);
   LCD_DISP_CHINESE_Char(0x09,0x00,0xC7E9);
   LCD_DISP_CHINESE_Char(0x09,0x04,0xC7E9);
#endif


#if   0    //  8x8 ascii
    LCD_DISP_ASCII_8X8Char(0,0,'A');  
    LCD_DISP_ASCII_8X8Char(0,15,'B');  
    LCD_DISP_ASCII_8X8Char(15,0,'C'); 
	LCD_DISP_ASCII_8X8Char(14,0,'D');  
	 LCD_DISP_ASCII_8X8Char(18,73,'Z'); 
	LCD_DISP_ASCII_8X8Char(19,73,'W');  
#endif



#if  0  //  8x16  ascii
    LCD_DISP_ASCII_8X16Char(0,0,'A');  
    LCD_DISP_ASCII_8X16Char(0,15,'B');  
	LCD_DISP_ASCII_8X16Char(11,32,'M');  
    LCD_DISP_ASCII_8X16Char(15,0,'C'); 
	LCD_DISP_ASCII_8X8Char(14,0,'D');  
	LCD_DISP_ASCII_8X16Char(18,66,'Z');  
	LCD_DISP_ASCII_8X16Char(19,64,'W');  

#endif


 #if  0          //画图
  for(i=0;i<20;i++)
  {
     for(j=0;j<i;j++)
       LCD_DISP_BitLatice(i,j);

  }
#endif 

  /*
  for(i=0;i<20;i++)
  {
     for(j=0;j<i;j++)
       LCD_DISP_ByteLatice(i+30,j+30,1);

  }
   */
  // delay_ms(20);
  // LCD_DISP_ASCII_8X16Char(20,35,'M');
  //  LCD_DISP_ASCII_8X16Char(40,35,'W');
 
   
}

#endif

static void msg( void *p)
{
}
static void show(void)
{ 
    Disp_Idle();
}


static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu:
		   rt_kprintf("\r\n==> Idle 菜单");
	       break;
    case KeyValueOk:  
           rt_kprintf("\r\n==> Idle OK 确认");
        break;
    case KeyValueUP:
		   rt_kprintf("\r\n==> Idle 向上");
        break;
    case KeyValueDown:
		  rt_kprintf("\r\n==>Idle 向下");

          break;
     case KeyValueQuit:
             rt_kprintf("\r\n==>Idle 退出--网络巡检");
			 LCD_DISP_Clear();
			 pMenuItem = &Menu_3_detect;
             pMenuItem->show();
              break;
	   case KeyValueApp:	
             rt_kprintf("\r\n==>Idle 应用-手动send");
			 LCD_DISP_Clear();
			 pMenuItem = &Menu_2_handle;
             pMenuItem->show();
		      break;
		
    }
    KeyValue = 0;
}

static void timetick(unsigned int systick)
{

         //循环显示待机界面
        tickcount++;
        if(tickcount >= 5)//5
        {
            tickcount = 0;
            Disp_Idle();
        }
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_1_Idle =
{
    "Menu_idle",
    8,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};

