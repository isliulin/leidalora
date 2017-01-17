#include  <string.h>
#include "Menu_Include.h"


static void msg( void *p)
{

}
static void show(void)
{
   u8  disp_len=0;
   
    MenuIdle_working = 0; //clear
    //rt_kprintf("\r\n------------------��ӡȱֽ----------");
    lcd_fill(0);
    switch(Menu_txt_state)
    {
    case 1:
        lcd_text12(20, 10, "��ӡȱֽ", 8, LCD_MODE_SET);
        break;
    case 2:
        lcd_text12(20, 10, "IC�����Ͳ�ƥ��", 14, LCD_MODE_SET);
        break;
    case 3:
        lcd_text12(20, 10, "�Ǳ�׼IC��", 10, LCD_MODE_SET);
        break;
    case 4:
        lcd_text12(0, 10, "USB�������ݵ������!", 20, LCD_MODE_SET);
        break;
    case 5:
        lcd_text12(20, 10, "��ӡ��...", 9, LCD_MODE_SET);
        break;
    case 6:    // ��ʾLORA
         switch(SysConf_struct.LORA_TYPE)
         	{
         	   case LORA_RELAYSTAION:    //  �м�
                                          lcd_text12(20, 0, "Relay Station", 13, LCD_MODE_SET);
			                             break;
			   case LORA_RADRCHECK:    //  �״����
                                          lcd_text12(20, 0, "Radar Check", 11, LCD_MODE_SET);  
			                             break;
			   case LORA_ENDPLAY:      //   ���ڲ��ŵ�
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


