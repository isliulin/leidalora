#include  <string.h>
#include "Menu_Include.h"


static void msg( void *p)
{

}
static void show(void)
{
   u8  disp_len=0;

   
   Menu_Number=4; 
   
    MenuIdle_working = 0; //clear
    //rt_kprintf("\r\n------------------��ӡȱֽ----------");
    lcd_fill(0);
    switch(Menu_txt_state)
    {
    case 1:
        lcd_text12(0, 10, "USB�������ݵ������!", 20, 1);
        break;
    case 5:
        lcd_text12(20, 10, "��ӡ��...", 9, 1);
        break;
    case 2:    // ��Я̨��ʾLORA
         	   //  ��Я�ն�
        lcd_text12(20, 0, "Carried Station", 15, 1); 

	   disp_len=strlen(LORA_RUN.Tx_Disp)-2;
	  
	  if(disp_len>18)
        lcd_text12(0, 10, (char*)LORA_RUN.Tx_Disp, 18, 1);   
	  else
        lcd_text12(0, 10, (char*)LORA_RUN.Tx_Disp, disp_len, 1);  
	  if(disp_len>18)	
		 lcd_text12(0, 22, (char *)LORA_RUN.Tx_Disp+18,(disp_len-18), 1);  
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
		              // LORA_RUN.SD_Enable=1;
		               break; 
    case KeyValueUP:
    case KeyValueDown:
       
                       break;
    }
    KeyValue = 0;
}


static void timetick(unsigned int systick)
{
    if(Menu_txt_state != 2)
    {
        CounterBack++;
        if(CounterBack != 30)
            return;
        pMenuItem = &Menu_1_Idle;
        pMenuItem->show();
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


