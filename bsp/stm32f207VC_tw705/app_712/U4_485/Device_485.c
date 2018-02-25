/*
    App Gsm uart
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



//---------------变量声明-------------------------------------------
//第四个字节  0x81: 320x240      0x82 : 640 x480
u8   Take_photo[10] = {0x40, 0x40, 0x61, 0x81, 0x02, 0X00, 0X00, 0X02, 0X0D, 0X0A}; ; //----  报警拍照命令
u8   Fectch_photo[10] = {0x40, 0x40, 0x62, 0x81, 0x02, 0X00, 0XFF, 0XFF, 0X0D, 0X0A};; //----- 报警取图命令






static  u16	 PackageLen = 0; //记录每次接收的byte数
u8  OpenDoor_StartTakeFlag = 0; // 车开关开始拍照状态 ，	开始拍照时	set   1   拍照结束后  0
u8   Opendoor_transFLAG = 0x02;	 //	车门打开拍照后是否上传标志位


//----------- _485 rx-----
ALIGN(RT_ALIGN_SIZE)
u8    _485_dev_rx[_485_dev_SIZE];
u16  _485dev_wr = 0;
u8   _485_outflag=0;
u8 	 _485_content[200];
u16	 _485_content_wr = 0;
u8  _485_speed=0;




struct rt_device  Device_485;
MSG_Q_TYPE  _485_MsgQue_sruct;
_485REC_Struct 	 _485_RXstatus;




//=====================================================


void _485_delay_ms(u16 j)
{
    while(j--)
    {
        delay_us(1000);
    }

}

/* write one character to serial, must not trigger interrupt */
void rt_hw_485_putc(const char c)
{
    USART_SendData(USART2,  c);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {}
}

void rt_hw_485_Output_Data(const char *Instr, unsigned int len)
{
    unsigned int  Info_len485 = 0;

    Info_len485 = len;
    /* empty console output */
    TX_485const;
    _485_delay_ms(1);
    //--------  add by  nathanlnw ---------
    while (Info_len485)
    {
        rt_hw_485_putc (*Instr++);
        Info_len485--;
    }
    //--------  add by  nathanlnw  --------
    _485_delay_ms(3);
    RX_485const;
}

void  _485_RxHandler(u8 data)
{
    //      Large   LCD   Data

    rt_interrupt_enter( );

   if(_485_outflag==0)
      _485_dev_rx[_485dev_wr++] = data;

   if(_485dev_wr>=4)
   {
	   if((_485_dev_rx[0]==0xFC)&&(_485_dev_rx[1]==0xFA)&&(_485_dev_rx[3]==0x00)&&(_485dev_wr==4))
	   	{
	   	   memcpy(_485_content,_485_dev_rx,4);
	       _485_content_wr=_485dev_wr;
		  
		  _485dev_wr=0;
	      _485_outflag=1;
	   	}
	   else
	   	{
	   	  _485dev_wr=0;
	      _485_outflag=0;
	   	}
   	}    

   /*
   if(data==0x0A)
    	{
            _485_outflag=1;
    	}
    */
    rt_interrupt_leave( );
}

void _485_process(void)
{
   if(_485_outflag) 
   	{
        // rt_kprintf("485RX:%s ",_485_dev_rx);
        // rt_hw_485_Output_Data(_485_dev_rx,_485dev_wr); // 返回给485
        OutPrint_HEX("\r\n 485 RX",_485_content,_485_content_wr);
		_485_speed=_485_content[2];

		//if(_485_speed>10)
            LORA_RUN.SD_Enable=1;

		
		rt_kprintf("\r\n  485 速度: %d  km/h",_485_speed);
       _485_outflag=0;
   	}

}



static rt_err_t   Device_485_init( rt_device_t dev )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;


    //  1 . Clock

    /* Enable USART2 and GPIOA clocks */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    /* Enable USART2 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //   2.  GPIO
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Connect alternate function */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    //  3.  Interrupt
    /* Enable the USART2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //   4.  uart  Initial
    USART_InitStructure.USART_BaudRate = 9600;  //485 
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(USART2, ENABLE);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    /* -------------- 485  操作相关 -----------------	*/
    /*
    	   STM32 Pin	 		   Remark
    		 PC4		  		    485_Rx_Tx 控制   0:Rx    1: Tx
    		 PD0		  		    485 电源	1: ON  0:OFF
    */

    /*  管脚初始化 设置为 推挽输出 */

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

      //------------------- PC4------------------------------
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;				 //--------- 485const	收发控制线
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    RX_485const;
    return RT_EOK;
}

static rt_err_t Device_485_open( rt_device_t dev, rt_uint16_t oflag )
{
    return RT_EOK;
}
static rt_err_t Device_485_close( rt_device_t dev )
{
    return RT_EOK;
}
static rt_size_t Device_485_read( rt_device_t dev, rt_off_t pos, void *buff, rt_size_t count )
{

    return RT_EOK;
}
static rt_size_t Device_485_write( rt_device_t dev, rt_off_t pos, const void *buff, rt_size_t count )
{
    unsigned int  Info_len485 = 0;
    const char		*p	= (const char *)buff;


    Info_len485 = (unsigned int)count;
    /* empty console output */
    TX_485const;
    _485_delay_ms(1);
    //--------  add by  nathanlnw ---------
    while (Info_len485)
    {
        rt_hw_485_putc (*p++);
        Info_len485--;
    }
    //--------  add by  nathanlnw  --------
    _485_delay_ms(3);
    RX_485const;
    return RT_EOK;
}
static rt_err_t Device_485_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
    return RT_EOK;
}

/* init 485 */
void _485_startup(void)
{

    Device_485.type	= RT_Device_Class_Char;
    Device_485.init	= Device_485_init;
    Device_485.open	=  Device_485_open;
    Device_485.close	=  Device_485_close;
    Device_485.read	=  Device_485_read;
    Device_485.write	=  Device_485_write;
    Device_485.control = Device_485_control;

    rt_device_register( &Device_485, "485", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE );
    rt_device_init( &Device_485 );

}


