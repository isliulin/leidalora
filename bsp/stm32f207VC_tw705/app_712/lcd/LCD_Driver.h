#ifndef SED1520_H_
#define SED1520_H_

// define to disable busy-delays (useful for simulation)
//// #define LCD_SIM (1)
// define to enable additional debugging functions
//// #define LCD_DEBUG (1)

#include <rtthread.h>
//#include <rtdevice.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>


#include  <stdlib.h>//数字转换成字符串
#include  <stdio.h>
#include  <string.h>
#include  "App_moduleConfig.h"
#include  "rtdevice.h "




#include <stdio.h>
#include <rtthread.h>
#include "stm32f2xx.h"
#include "board.h"
//#include "ringbuffer.h"
#include <finsh.h>


#include <stdint.h>
#include "bmp.h"

//   LCD  





extern void LCD_init(void);
extern void LCD_DISP_8x8_TXT(u8 Row, u8 Colum, u8 *p, u8 len);
extern void LCD_DISP_8x16_TXT(u8 Row, u8 Colum, u8 *p, u8 len);
extern void LCD_DISP_CHINESE_TXT(u8 Row, u8 Colum,u8 *p, u8 len);
extern u16   GB2312_to_LCD_Code(u16 inValue);
extern void  LCD_DISP_ASCII_8X8Char(u8 Row, u8 Colum,u8  AsciiValue);
extern void  LCD_DISP_ASCII_8X16Char(u8 Row, u8 Colum,u8  AsciiValue);
extern void  LCD_DISP_CHINESE_Char(u8 Row, u8 Colum,u16  ChineseCode);
extern void  LCD_DISP_BitLatice(u8 Row, u8 Colum);
extern void  LCD_DISP_ByteLatice(u8 Row, u8 Colum,u8 option);   // option   0: 白点   1 黑点
extern void  LCD_DISP_Clear(void);




















extern void lcd_fill(const unsigned char pattern);

extern void lcd_update(const unsigned char top, const unsigned char bottom);
extern void lcd_update_all(void);


#if 0
extern void lcd_rect( const uint8_t x, const uint8_t y, uint8_t width, uint8_t height, const uint8_t mode);
extern void lcd_box(const uint8_t x, const uint8_t y, uint8_t width, const uint8_t height, const uint8_t mode);
extern void lcd_circle(const uint8_t xCenter, const uint8_t yCenter, const uint8_t radius, const uint8_t mode);
#endif
extern void lcd_text12(char left, char top , char *p, char len, const char mode);
extern void lcd_text12_local(char left, char top , char *p, char len, const char mode);
extern void LCD_RstLow(void);




extern void Lcd_hardInit_timer(void);

#endif
