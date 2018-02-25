/**************************************************************************
 *
 *   LCD_Driver.c
 *   LCD display controller interface routines for graphics modules
 *   with onboard LCD_Driver controller(s) in "write-only" setup
 *
 *   Version 1.02 (20051031)
 *
 *   For Atmel AVR controllers with avr-gcc/avr-libc
 *   Copyright (c) 2005
 *     Martin Thomas, Kaiserslautern, Germany
 *     <eversmith@heizung-thomas.de>
 *     http://www.siwawi.arubi.uni-kl.de/avr_projects
 *
 *   Permission to use in NON-COMMERCIAL projects is herbey granted. For
 *   a commercial license contact the author.
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *   FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *   COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *   partly based on code published by:
 *   Michael J. Karas and Fabian "ape" Thiele
 *
 *
 ***************************************************************************/

/*
   An Emerging Display EW12A03LY 122x32 Graphics module has been used
   for testing. This module only supports "write". There is no option
   to read data from the LCD_Driver RAM. The LCD_Driver R/W line on the
   module is bound to GND according to the datasheet. Because of this
   Read-Modify-Write using the LCD-RAM is not possible with the 12A03
   LCD-Module. So this library uses a "framebuffer" which needs
   ca. 500 bytes of the AVR's SRAM. The libray can of cause be used
   with read/write modules too.
*/

/* tab-width: 4 */

//#include <LPC213x.H>
//#include <includes.h>

#include <stdint.h>
#include "LCD_Driver.h"
#include "bmp.h"
#include "board.h"
#include "stm32f2xx.h"
#include "App_moduleConfig.h"

/* pixel level bit masks for display */
/* this array is setup to map the order */
/* of bits in a byte to the vertical order */
/* of bits at the LCD controller */
const unsigned char l_mask_array[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}; /* TODO: avoid or PROGMEM */

/* the LCD display image memory */
/* buffer arranged so page memory is sequential in RAM */

//  new   Lcd MMD
unsigned char ADJUST = 0x23;	//对比度寄存器初始值	，vop调整设置  当前设置数值VDD=3.0V,VOP=0X10=9V


/* control-lines hardware-interface (only "write") */
#define LCD_CMD_MODE()     LCDCTRLPORT &= ~(1<<LCDCMDPIN)
#define LCD_DATA_MODE()    LCDCTRLPORT |=  (1<<LCDCMDPIN)
#define LCD_ENABLE_E1()    LCDCTRLPORT &= ~(1<<LCDE1PIN)
#define LCD_DISABLE_E1()   LCDCTRLPORT |=  (1<<LCDE1PIN)
#define LCD_ENABLE_E2()    LCDCTRLPORT &= ~(1<<LCDE2PIN)
#define LCD_DISABLE_E2()   LCDCTRLPORT |=  (1<<LCDE2PIN)




#define MR		(1<<15)
#define SHCP	(1<<12)
#define DS		(1<<14)
#define STCP1	(1<<15)
#define STCP2	(1<<13)

//--- ST7565R --------
#define RST0	(1<<0)
#define CS_		(1<<1)   // new  LCD     /CS
#define RD_		(1<<2)   // new LCD      /RD
#define WR_		(1<<3)
#define A0		(1<<4)
//----- SED1520  ---------------
#define E1		(1<<1)
#define E2		(1<<2)
#define RW		(1<<3)




#define pgm_read_byte(a)	(*(a))
#define pgm_read_word(a) (*(a))


//  设C9E8
u8 Dot_She[24] = {0x10, 0x00, 0x11, 0x00, 0xF2, 0x07, 0x00, 0x02, 0x50, 0x08, 0xCF, 0x08, 0x41, 0x05, 0x41, 0x02, 0x4F, 0x05, 0xD0, 0x08, 0x10, 0x08, 0x00, 0x00};


// 备B1B8
u8 Dot_Bei[24] = {0x20, 0x00, 0x24, 0x00, 0xD2, 0x0F, 0x57, 0x05, 0x4A, 0x05, 0xCA, 0x07, 0x4A, 0x05, 0x56, 0x05, 0xD2, 0x0F, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00};

//自 D7D4
u8 Dot_Zi[24] = {0x00, 0x00, 0xFC, 0x0F, 0x24, 0x09, 0x24, 0x09, 0x26, 0x09, 0x25, 0x09, 0x24, 0x09, 0x24, 0x09, 0x24, 0x09, 0xFC, 0x0F, 0x00, 0x00, 0x00, 0x00};

//检 BCEC
u8 Dot_Jian[24] = {0x88, 0x00, 0x68, 0x00, 0xFF, 0x0F, 0x48, 0x00, 0x10, 0x09, 0x28, 0x0E, 0xA4, 0x08, 0x23, 0x0B, 0x24, 0x0C, 0xA8, 0x0B, 0x10, 0x08, 0x00, 0x00};


// 开BFAA
u8  Dot_Kai[24] = {0x40, 0x00, 0x42, 0x08, 0x42, 0x06, 0xFE, 0x01, 0x42, 0x00, 0x42, 0x00, 0x42, 0x00, 0xFE, 0x0F, 0x42, 0x00, 0x42, 0x00, 0x40, 0x00, 0x00, 0x00};



// 正D5FD
u8 Dot_Zheng[24] = {0x00, 0x08, 0x02, 0x08, 0xE2, 0x0F, 0x02, 0x08, 0x02, 0x08, 0xFE, 0x0F, 0x42, 0x08, 0x42, 0x08, 0x42, 0x08, 0x42, 0x08, 0x00, 0x08, 0x00, 0x00};

//  常 B3A3
u8 Dot_Chang[24] = {0x0C, 0x00, 0x04, 0x07, 0x75, 0x01, 0x56, 0x01, 0x54, 0x01, 0xD7, 0x0F, 0x54, 0x01, 0x56, 0x01, 0x75, 0x05, 0x04, 0x07, 0x0C, 0x00, 0x00, 0x00};

// 天CCEC
u8 Dot_Tian[24] = {0x20, 0x08, 0x22, 0x08, 0x22, 0x04, 0x22, 0x02, 0xA2, 0x01, 0x7E, 0x00, 0xA2, 0x01, 0x22, 0x02, 0x22, 0x04, 0x22, 0x08, 0x20, 0x08, 0x00, 0x00};

// 线CFDF
u8 Dot_Xian[24] = {0x98, 0x04, 0xD4, 0x04, 0xB3, 0x02, 0x88, 0x02, 0x00, 0x08, 0x48, 0x08, 0x48, 0x04, 0xFF, 0x03, 0x24, 0x05, 0xA5, 0x08, 0x26, 0x0E, 0x00, 0x00};

//短C2B7
u8 Dot_Duan [24] = {0x9E, 0x0F, 0x12, 0x08, 0xF2, 0x07, 0x9E, 0x04, 0x48, 0x00, 0xC4, 0x0F, 0xAB, 0x04, 0x92, 0x04, 0xAA, 0x04, 0xC6, 0x0F, 0x40, 0x00, 0x00, 0x00};

//路 B6CC
u8 Dot_Lu[24] = {0x48, 0x08, 0x47, 0x06, 0xFC, 0x01, 0x44, 0x06, 0x02, 0x08, 0x7A, 0x09, 0x4A, 0x0A, 0x4A, 0x08, 0x4A, 0x0A, 0x7A, 0x09, 0x02, 0x08, 0x00, 0x00};

//  内C4DA
u8 Dot_Nei[24] = {0x00, 0x00, 0xFC, 0x0F, 0x04, 0x00, 0x84, 0x00, 0x64, 0x00, 0x1F, 0x00, 0x24, 0x00, 0xC4, 0x08, 0x04, 0x08, 0xFC, 0x0F, 0x00, 0x00, 0x00, 0x00};

// 部B2BF
u8 Dot_Bu[24] = {0x20, 0x00, 0xAA, 0x0F, 0xB2, 0x04, 0xA3, 0x04, 0xB2, 0x04, 0xAA, 0x0F, 0x20, 0x00, 0xFE, 0x0F, 0x02, 0x02, 0x32, 0x02, 0xCE, 0x01, 0x00, 0x00};

// 电B5E7
u8 Dot_Dian[24] = {0xFC, 0x03, 0x24, 0x01, 0x24, 0x01, 0x24, 0x01, 0xFF, 0x07, 0x24, 0x09, 0x24, 0x09, 0x24, 0x09, 0xFC, 0x09, 0x00, 0x08, 0x00, 0x0E, 0x00, 0x00};

// 池B3D8
u8 Dot_Chi[24] = {0x22, 0x04, 0x44, 0x02, 0x40, 0x00, 0xFC, 0x07, 0x20, 0x08, 0x10, 0x08, 0xFF, 0x0B, 0x08, 0x08, 0x04, 0x09, 0xFC, 0x09, 0x00, 0x0C, 0x00, 0x00};

// 供B9A9
u8 Dot_Gong[24] = {0x10, 0x00, 0xFC, 0x0F, 0x83, 0x08, 0x88, 0x04, 0xFF, 0x02, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0xFF, 0x02, 0x88, 0x04, 0x80, 0x08, 0x00, 0x00};

// .
u8 Dot_dot[24] = {0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x20, 0x01, 0x20, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


u8   lcd_hard_init = 0; // 每屏幕  显示器硬件初始话
u32  lcd_hardIint_couter = 0;




void  LCD_Wrie_Drv(u8 ValueByte)
{
  u8 i=0;
   
   for(i=0;i<8;i++)
   	{
   	   if((ValueByte>>i)&0x01)
   	   	{
   	   	    switch(i)
   	   	    	{
                   case 0:   LCD_DB0_1;
							 break;
				   case 1:   LCD_DB1_1;
							 break;
				   case 2:   LCD_DB2_1;
							 break;
									
				   case 3:   LCD_DB3_1;
							 break;

				   case 4:   LCD_DB4_1;
							 break;
				   	
				   case 5:   LCD_DB5_1;
							 break;
				   	
				   case 6:   LCD_DB6_1;
							 break;
				   	
				   case 7:   LCD_DB7_1;
							 break;

   	   	    	}


   	   	}
	   else
	   	{
		    switch(i)
			   {
				  case 0:    LCD_DB0_0;
				             break;				           
				  case 1:    LCD_DB1_0;
				             break;	
		   
				  case 2:    LCD_DB2_0;
				             break;	 
								   
				  case 3:    LCD_DB3_0;
				             break;	
		   
 				  case 4:    LCD_DB4_0;
				             break;	
 				   
				  case 5:    LCD_DB5_0;
				             break;	
				   
				  case 6:    LCD_DB6_0;
				             break;	
				   
				  case 7:    LCD_DB7_0;
				             break;	
		   
			   }

	   	}

   	}

}

void  LCD_init(void)
{
     LCD_REQ_0;
	 LCD_RES_1;
}

void LCD_RstLow(void)
{
   LCD_RES_0;
}

void LCD_WriteByte(u8 ByteValue)
{   
  u16  i=3000;
  // 0   wait when  BUSY
   if(GPIO_ReadInputDataBit(LCD_BUSY_PORT, LCD_BUSY_PIN))
    {
      while(i--)
          ;
	  if(GPIO_ReadInputDataBit(LCD_BUSY_PORT, LCD_BUSY_PIN))
	  	  return;
	}
   // 1  write byte content   when  busy==0
     LCD_Wrie_Drv(ByteValue);
   // 2. delay  NOP
      delay_us(1);
   // 3. set  REQ
      LCD_REQ_1;
   // 4. wait busy=1
      i=3000;
      while(i--)
      	{
      	  if(GPIO_ReadInputDataBit(LCD_BUSY_PORT, LCD_BUSY_PIN))
             break;
      	}
   //  5.  clear  REQ
     LCD_REQ_0;
   //  6.  wait  to end
      delay_us(2);

}

void  LCD_DISP_CHINESE_Char(u8 Row, u8 Colum,u16  ChineseCode) // 列  行
{    //  发送5 个字节   
     u16   DchineseCode=0;
     DchineseCode=GB2312_to_LCD_Code(ChineseCode);
     LCD_WriteByte(0xF0); // 汉字
     LCD_WriteByte(Row);
	 LCD_WriteByte(Colum);
     LCD_WriteByte((u8)(DchineseCode>>8));	 
     LCD_WriteByte((u8)DchineseCode);
	 delay_ms(3);
}

void  LCD_DISP_ASCII_8X8Char(u8 Row, u8 Colum,u8  AsciiValue)   // 列  行
{    
     LCD_WriteByte(0xF1);
     LCD_WriteByte(Row);
	 LCD_WriteByte(Colum);
     LCD_WriteByte(AsciiValue);	 
	 delay_ms(3);
}

void  LCD_DISP_ASCII_8X16Char(u8 Row, u8 Colum,u8  AsciiValue)   // 列  行
{  
     LCD_WriteByte(0xF9); 
     LCD_WriteByte(Row);
	 LCD_WriteByte(Colum);
     LCD_WriteByte(AsciiValue);	 
	 delay_ms(3);
}

void  LCD_DISP_BitLatice(u8 Row, u8 Colum)
{   
     LCD_WriteByte(0xF2); 
     LCD_WriteByte(Row);
	 LCD_WriteByte(Colum);
	 delay_us(300);
}

void  LCD_DISP_ByteLatice(u8 Row, u8 Colum,u8 option)   // option   0: 白点   1 黑点
{   
     LCD_WriteByte(0xF3); 
     LCD_WriteByte(Row);
	 LCD_WriteByte(Colum);
	 LCD_WriteByte(option);
	 delay_us(300);
}

void  LCD_DISP_Clear(void)
{   
     LCD_WriteByte(0xF4); 
	 delay_ms(200);
}
FINSH_FUNCTION_EXPORT(LCD_DISP_Clear, LCD_DISP_Clear());


void  LCD_DISP_MoveUp(void)
{   
     LCD_WriteByte(0xF5); //0xF5
}
FINSH_FUNCTION_EXPORT(LCD_DISP_MoveUp, LCD_DISP_MoveUp());

void  LCD_DISP_MoveDown(void)
{   
     LCD_WriteByte(0xF6); 
}

void  LCD_DISP_MoveLeft(void)
{   
     LCD_WriteByte(0xF7); 
}

void  LCD_DISP_MoveRight(void)
{   
     LCD_WriteByte(0xF8); 
}

void  LCD_DISP_Reverse(void)
{   
     LCD_WriteByte(0xFA); 
}

void  LCD_DISP_Cursor(u8 select)    //  00  not disp cursor  ;  07   8bit  cursor ;  0F   16bit cursor  
{   
     LCD_WriteByte(0xFB); 
	 LCD_WriteByte(select);
}

void  LCD_DISP_MoveSPD(u8 select)    //  00  move 1 point  ;  01: move 2point  ;   07  move 8point  ;  0F   16point  
{   
     LCD_WriteByte(0xFC); 
	 LCD_WriteByte(select);
}

u16 GB2312_to_LCD_Code(u16 inValue)       
{
    u16  RTN[2];
    u16  RTN_value=0;
   
	 RTN[0]=(inValue>>8)-0xA0;
     RTN[1]=(u8)(inValue)-0xA0; 
     RTN_value=(RTN[0]<<8)+RTN[1];
	 return RTN_value;
}


void LCD_DISP_8x8_TXT(u8 Row, u8 Colum, u8 *p, u8 len)   //  73   20
{
    u8  rrow=Colum,ccolum=Row;
	u8  i=0;

     if((Colum>LCD_88_Lmax)||(Row>LCD_88_Hmax))
		 return;
	 

	for(i=0;i<len;i++)
    {
        LCD_DISP_ASCII_8X8Char(rrow,ccolum,*p++);
		if(rrow>=(LCD_88_Lmax-1))  //  如果一行满了，自动切换到下一行
		{
              rrow=0;
			  ccolum+=8;
			  if(ccolum>LCD_88_Hmax)
			  	 ccolum=0;
		}
		else
			rrow++;
	}

}

void LCD_DISP_8x16_TXT(u8 Row, u8 Colum, u8 *p, u8 len)   //20  66
{
    u8  rrow=Colum,ccolum=Row;
	u8  i=0;

    if((Colum>LCD_816_Lmax)||(Row>LCD_816_Hmax))
		 return;
	

	for(i=0;i<len;i++)
    {
        LCD_DISP_ASCII_8X16Char(rrow,ccolum,*p++);
		if(rrow>=(LCD_816_Lmax-1))  //  如果一行满了，自动切换到下一行
		{
             rrow=0;//L++
             ccolum+=16;
			  if(ccolum>LCD_816_Hmax)
			  	return;
			     
		}
		else
			rrow++;
	}



}

void LCD_DISP_CHINESE_TXT(u8 Row, u8 Colum,u8 *p, u8 len)
{
    u8  rrow=Colum,ccolum=Row;
	u8  i=0,hz_len=0,reg2b[2];
	u16 HZ_value=0;

    if(len%2)
		return;
     if((Colum>LCD_HZ_Lmax)||(Row>LCD_HZ_Hmax))
		 return;
	

    hz_len=len>>1;  

	for(i=0;i<hz_len;i++)
    {
        reg2b[0]=*p++;
		reg2b[1]=*p++;
		HZ_value=(reg2b[0]<<8)+reg2b[1];
		
        LCD_DISP_CHINESE_Char(rrow,ccolum,HZ_value);
		if(rrow>(LCD_HZ_Lmax-1))  //  如果一行满了，自动切换到下一行
		{
		      rrow=0;
              ccolum++;
			  if(ccolum>LCD_HZ_Hmax)
			  	 return; 
		}
		else
			rrow++;
	}

}







/*
**
** routine to initialize the operation of the LCD display subsystem
**
*/


/* fill buffer and LCD with pattern */
void lcd_fill(const unsigned char pattern)
{

}


/*
**
** Updates area of the display. Writes data from "framebuffer"
** RAM to the lcd display controller RAM.
**
** Arguments Used:
**    top     top line of area to update.
**    bottom  bottom line of area to update.
**    from MJK-Code
**
*/
void lcd_update(const unsigned char top, const unsigned char bottom)
{
}



void lcd_update_all(void)
{
}


/*
绘制12点阵的字符，包括中文和英文


*/
void lcd_text12(char left, char top , char *p, char len, const char mode)
{

}



/*
      ST7565R  LCD  定时初始化液晶一次     放到1s 定时器那里
*/
void Lcd_hardInit_timer(void)
{
    if(0x01 == HardWareVerion) // 新的硬件版本
    {
        lcd_hardIint_couter++;
        if(lcd_hardIint_couter > 3600) //3600
        {
            lcd_hard_init = 1;
            lcd_hardIint_couter = 0;
        }
    }
    else if(0x03 == HardWareVerion)
        lcd_hard_init = 1;
}

