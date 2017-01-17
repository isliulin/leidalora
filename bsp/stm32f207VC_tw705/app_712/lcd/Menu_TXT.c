#include  <string.h>
#include "Menu_Include.h"


static void msg( void *p)
{

}
static void show(void)
{
   u8  disp_len=0;
   
    MenuIdle_working = 0; //clear
    //rt_kprintf("\r\n------------------打印缺纸----------");
    lcd_fill(0);
    switch(Menu_txt_state)
    {
    case 1:
        lcd_text12(20, 10, "打印缺纸", 8, LCD_MODE_SET);
        break;
    case 2:
        lcd_text12(20, 10, "IC卡类型不匹配", 14, LCD_MODE_SET);
        break;
    case 3:
        lcd_text12(20, 10, "非标准IC卡", 10, LCD_MODE_SET);
        break;
    case 4:
        lcd_text12(0, 10, "USB所有数据导出完成!", 20, LCD_MODE_SET);
        break;
    case 5:
        lcd_text12(20, 10, "打印中...", 9, LCD_MODE_SET);
        break;
    case 6:    // 显示LORA
         switch(SysConf_struct.LORA_TYPE)
         	{
         	   case LORA_RELAYSTAION:    //  中继
                                          lcd_text12(20, 0, "Relay Station", 13, LCD_MODE_SET);
			                             break;
			   case LORA_RADRCHECK:    //  雷达监测点
                                          lcd_text12(20, 0, "Radar Check", 11, LCD_MODE_SET);  
			                             break;
			   case LORA_ENDPLAY:      //   道口播放点
                                          lcd_text12(20, 0, "Road Play", 9, LCD_MODE_SET);   
			                             break;		
			   default: break;			 

         	}
	  if(SysConf_struct.LORA_TYPE==LORA_RADRCHECK) 	 
	      disp_len=strlen(LORA_RUN.Tx_Disp)-2;
	  else
          disp_len=strlen(LORA_RUN.Tx_Disp);
	  
	  if(disp_len>18)
        lcd_text12(0, 10, (char*)LORA_RUN.Tx_Disp, 18, LCD_MODE_SET);   
	  else
        lcd_text12(0, 10, (char*)LORA_RUN.Tx_Disp, disp_len, LCD_MODE_SET);  
	  if(disp_len>18)	
		 lcd_text12(0, 22, (char *)LORA_RUN.Tx_Disp+18,(disp_len-18), LCD_MODE_SET);  
        break;	 	 
    }
    lcd_update_all(); 

    Menu_txt_state = 0; // clear
}


static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu: 
		               pMenuItem = &Menu_1_Idle;
                       pMenuItem->show();
		               break;
    case KeyValueOk: 
		               LORA_RUN.SD_Enable=1;
		               break; 
    case KeyValueUP:
    case KeyValueDown:
       
                       break;
    }
    KeyValue = 0;
}


static void timetick(unsigned int systick)
{
    if(Menu_txt_state != 6)
    {
        CounterBack++;
        if(CounterBack != 30)
            return;
       // pMenuItem = &Menu_1_Idle;
       // pMenuItem->show();
        CounterBack = 0;
    }
}


MENUITEM	Menu_TXT =
{
    "Menu_TXT",
    8,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};


