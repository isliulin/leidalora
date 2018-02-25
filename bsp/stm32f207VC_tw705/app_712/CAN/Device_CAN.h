/*
     Device_CAN.h
*/
#ifndef  _RT_Dev_CAN
#define _RT_Dev_CAN

#include <rthw.h>
#include <rtthread.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>




#define CANTESTInit 1
#define CANTESTTX 1
#define CANTESTRX 0

#define CANBufSize   100

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

extern TestStatus TestRx;
extern u8	CAN_initOver;
extern CanRxMsg RxMessageData;

extern u32   CANBuffer_wr;//--------- Debug ----------
extern u32   Can_RXnum;
extern u32   Can_sdnum;
extern u32   Can_same;
extern u32   Can_loudiao;
extern u32   Can_notsame;








#endif

