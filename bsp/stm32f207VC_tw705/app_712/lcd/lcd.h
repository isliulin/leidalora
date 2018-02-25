#ifndef LCDDis_H_
#define LCDDis_H_

#include "LCD_Driver.h"
#include <stdio.h>


// note    703    按键顺序:       菜单   确认  上翻  下翻(打印)
/*
#define KEY_MENU_PORT	GPIOC
#define KEY_MENU_PIN	GPIO_Pin_8

#define KEY_OK_PORT		GPIOA
#define KEY_OK_PIN		GPIO_Pin_8

#define KEY_UP_PORT		GPIOC
#define KEY_UP_PIN		GPIO_Pin_9

#define KEY_DOWN_PORT	GPIOD
#define KEY_DOWN_PIN	GPIO_Pin_3
*/

// note    便携台  按键顺序:       菜单   上    下   确认   退出 备用

#define KEY_MENU_PORT	GPIOE
#define KEY_MENU_PIN	GPIO_Pin_10

#define KEY_OK_PORT		GPIOE
#define KEY_OK_PIN		GPIO_Pin_9

#define KEY_UP_PORT		GPIOE
#define KEY_UP_PIN		GPIO_Pin_12

#define KEY_DOWN_PORT	GPIOE
#define KEY_DOWN_PIN	GPIO_Pin_11

#define KEY_RTN_BACK_PORT   GPIOE
#define KEY_RTN_BACK_PIN    GPIO_Pin_8

#define KEY_BAKUSE_PORT   GPIOE
#define KEY_BAKUSE_PIN    GPIO_Pin_7



//  新型号液晶屏
/*      
     厂家:  金鹏电子     型号:  OCMJ5X10A/B        点阵是:   160X80
*/

#define LCD_X_MAX    160
#define LCD_Y_MAX    80


#define LCD_88_Hmax      73 
#define LCD_88_Lmax      20


#define LCD_816_Hmax     66 
#define LCD_816_Lmax     20

#define LCD_HZ_Hmax       4 
#define LCD_HZ_Lmax      10  






#define   LCD_DB7_PORT    GPIOD
#define   LCD_DB7_PIN     GPIO_Pin_9

#define   LCD_DB6_PORT    GPIOD
#define   LCD_DB6_PIN     GPIO_Pin_10

#define   LCD_DB5_PORT    GPIOD
#define   LCD_DB5_PIN     GPIO_Pin_11

#define   LCD_DB4_PORT    GPIOD
#define   LCD_DB4_PIN     GPIO_Pin_12

#define   LCD_DB3_PORT    GPIOD
#define   LCD_DB3_PIN     GPIO_Pin_13

#define   LCD_DB2_PORT    GPIOD
#define   LCD_DB2_PIN     GPIO_Pin_14

#define   LCD_DB1_PORT    GPIOD
#define   LCD_DB1_PIN     GPIO_Pin_15

#define   LCD_DB0_PORT    GPIOC
#define   LCD_DB0_PIN     GPIO_Pin_6

#define   LCD_BUSY_PORT   GPIOC              //input
#define   LCD_BUSY_PIN    GPIO_Pin_7

#define   LCD_RES_PORT    GPIOC
#define   LCD_RES_PIN     GPIO_Pin_8

#define   LCD_REQ_PORT    GPIOC
#define   LCD_REQ_PIN     GPIO_Pin_9

#define   LCD_LED_PORT    GPIOA
#define   LCD_LED_PIN     GPIO_Pin_8 


// control
#define   LCD_DB7_1        GPIO_SetBits(LCD_DB7_PORT, LCD_DB7_PIN)
#define   LCD_DB7_0        GPIO_ResetBits(LCD_DB7_PORT, LCD_DB7_PIN)

#define   LCD_DB6_1        GPIO_SetBits(LCD_DB6_PORT, LCD_DB6_PIN)
#define   LCD_DB6_0        GPIO_ResetBits(LCD_DB6_PORT, LCD_DB6_PIN)


#define   LCD_DB5_1        GPIO_SetBits(LCD_DB5_PORT, LCD_DB5_PIN)
#define   LCD_DB5_0        GPIO_ResetBits(LCD_DB5_PORT, LCD_DB5_PIN)


#define   LCD_DB4_1        GPIO_SetBits(LCD_DB4_PORT, LCD_DB4_PIN)
#define   LCD_DB4_0        GPIO_ResetBits(LCD_DB4_PORT, LCD_DB4_PIN)


#define   LCD_DB3_1        GPIO_SetBits(LCD_DB3_PORT, LCD_DB3_PIN)
#define   LCD_DB3_0        GPIO_ResetBits(LCD_DB3_PORT, LCD_DB3_PIN)

#define   LCD_DB2_1        GPIO_SetBits(LCD_DB2_PORT, LCD_DB2_PIN)
#define   LCD_DB2_0        GPIO_ResetBits(LCD_DB2_PORT, LCD_DB2_PIN)


#define   LCD_DB1_1        GPIO_SetBits(LCD_DB1_PORT, LCD_DB1_PIN)
#define   LCD_DB1_0        GPIO_ResetBits(LCD_DB1_PORT, LCD_DB1_PIN)

#define   LCD_DB0_1        GPIO_SetBits(LCD_DB0_PORT, LCD_DB0_PIN)
#define   LCD_DB0_0        GPIO_ResetBits(LCD_DB0_PORT, LCD_DB0_PIN)

//-----
#define   LCD_RES_1        GPIO_SetBits(LCD_RES_PORT, LCD_RES_PIN)
#define   LCD_RES_0        GPIO_ResetBits(LCD_RES_PORT, LCD_RES_PIN)

#define   LCD_REQ_1        GPIO_SetBits(LCD_REQ_PORT, LCD_REQ_PIN)
#define   LCD_REQ_0        GPIO_ResetBits(LCD_REQ_PORT, LCD_REQ_PIN)

#define   LCD_LED_ON        GPIO_SetBits(LCD_LED_PORT, LCD_LED_PIN)
#define   LCD_LED_OFF        GPIO_ResetBits(LCD_LED_PORT, LCD_LED_PIN)



extern void KeyCheckFun(void);
extern void Init_lcdkey(void);


#endif
