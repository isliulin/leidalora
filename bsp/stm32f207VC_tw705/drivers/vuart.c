/*
串口网关，类似一个软串口
finsh和rt_printf的重定向
*/
#include <rtthread.h>
#include <rtdevice.h>
#include "serial.h"
#include <stm32f2xx_usart.h>
#include <finsh.h>
#include "App_moduleConfig.h"


#define UART1_GPIO_TX		GPIO_Pin_9
#define UART1_TX_PIN_SOURCE GPIO_PinSource9
#define UART1_GPIO_RX		GPIO_Pin_10
#define UART1_RX_PIN_SOURCE GPIO_PinSource10
#define UART1_GPIO			GPIOA
#define UART1_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define RCC_APBPeriph_UART1	RCC_APB2Periph_USART1

#define VUART_RXBUF_SIZE	1024
static uint8_t vuart_rxbuf[VUART_RXBUF_SIZE];

static struct rt_ringbuffer rb_vuart;
struct rt_device dev_vuart;


#define BD_SYNC_40	0
#define BD_SYNC_0D	1
#define BD_SYNC_0A	2

static u8 rx_fliter = 0; //   接收区分


void USART1_IRQHandler(void)
{
    rt_uint8_t ch;
    rt_interrupt_enter();

    if( USART_GetITStatus( USART1, USART_IT_RXNE ) != RESET )
    {
        ch = USART_ReceiveData( USART1 );
        USART_ClearITPendingBit( USART1, USART_IT_RXNE );
        if(GB19056.workstate == 0)
        {
            rt_ringbuffer_putchar(&rb_vuart, ch);
            //if (dev_vuart.rx_indicate != RT_NULL)  /*默认finsh使用*/
            {
                dev_vuart.rx_indicate(&dev_vuart, 1);
            }
        }
        else
        {
            GB19056.rx_flag = 1;
            GB19056.u1_rx_timer = 0;
            GB19056.rx_buf[GB19056.rx_wr++] = ch;


            //------------------------------------------
            if(GB19056.rx_buf[0] == 0xAA)
                rx_fliter = 1;
            else if( GB19056.rx_buf[0] == 'd')
                rx_fliter = 2;
            else
            {
                rx_fliter = 0;
                GB19056.rx_wr = 0;
            }
            //-------------------------------------------

            switch(rx_fliter)
            {
            case 1:   //  记录仪协议
                if(GB19056.rx_wr >= 6)
                {
                    if( GB19056.rx_buf[1] != 0x75)
                        GB19056.rx_wr = 0;
                    {
                        GB19056.rx_infoLen = (GB19056.rx_buf[3] << 8) + GB19056.rx_buf[4];

                        if(GB19056.rx_wr >= (GB19056.rx_infoLen + 7)) // 6head  +1 fcs
                        {
                            if(GB19056.rx_wr == (GB19056.rx_infoLen + 7))
                            {
                                rt_sem_release(&GB_RX_sem);
                                rx_fliter = 0;
                            }
                        }
                    }

                }
                break;
            case 2:  // 设置终端ID      deviceid("012345678901")\r\n
                if(GB19056.rx_wr >= 26)
                {
                    if(strncmp(GB19056.rx_buf, "deviceid", 8) == 0)
                    {
                        rt_sem_release(&GB_RX_sem);
                        rx_fliter = 0;
                    }
                    else
                    {
                        rx_fliter = 0;
                        GB19056.rx_wr = 0;
                    }

                }
                break;

            }


        }

    }
    rt_interrupt_leave();

}

void uart1_baud(int buad)
{
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = buad;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//USART_WordLength_9b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;  //USART_Parity_Odd;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
}
//FINSH_FUNCTION_EXPORT( uart1_baud, uart1 baud );


/*初始化串口1*/
static rt_err_t dev_vuart_init( rt_device_t dev )
{
    GPIO_InitTypeDef	GPIO_InitStructure;
    NVIC_InitTypeDef	NVIC_InitStructure;
    //	USART_InitTypeDef	USART_InitStructure;

    RCC_AHB1PeriphClockCmd(UART1_GPIO_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APBPeriph_UART1, ENABLE);

    /*uart1 管脚设置*/
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX | UART1_GPIO_TX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);
    GPIO_PinAFConfig(UART1_GPIO, UART1_TX_PIN_SOURCE, GPIO_AF_USART1);
    GPIO_PinAFConfig(UART1_GPIO, UART1_RX_PIN_SOURCE, GPIO_AF_USART1);

    /*NVIC 设置*/
    NVIC_InitStructure.NVIC_IRQChannel						= USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 3;//3//
    NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    uart1_baud(115200); 

    USART_Cmd( USART1, ENABLE );
    USART_ClearFlag(USART1, USART_FLAG_TC);//
    USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );

    return RT_EOK;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t dev_vuart_open( rt_device_t dev, rt_uint16_t oflag )
{
    return RT_EOK;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t dev_vuart_close( rt_device_t dev )
{

    return RT_EOK;
}

/***********************************************************
* Function:gps_read
* Description:数据模式下读取数据
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_size_t dev_vuart_read( rt_device_t dev, rt_off_t pos, void *buff, rt_size_t count )
{
    if(count > 1)
        return rt_ringbuffer_get(&rb_vuart, buff, count);
    else
        return rt_ringbuffer_getchar(&rb_vuart, buff);

}

/*
static void uart1_putc( const char c )
{
	USART_SendData( USART1, c );
	while( !( USART1->SR & USART_FLAG_TXE ) )
	{
		;
	}
	USART1->DR = ( c & 0x1FF );
}
*/

/***********************************************************
* Function:		gps_write
* Description:	数据模式下发送数据，要对数据进行封装
* Input:		const void* buff	要发送的原始数据
       rt_size_t count	要发送数据的长度
       rt_off_t pos		使用的socket编号
* Output:
* Return:
* Others:
***********************************************************/

static rt_size_t dev_vuart_write( rt_device_t dev, rt_off_t pos, const void *buff, rt_size_t count )
{
    rt_size_t	size = count;
    uint8_t		*ptr	= (uint8_t *)buff;

    if (dev_vuart.flag & RT_DEVICE_FLAG_STREAM)
    {
        while (size)
        {
            if (*ptr == '\n')
            {
                while (!(USART1->SR & USART_FLAG_TXE));
                USART1->DR = '\r';
            }

            while (!(USART1->SR & USART_FLAG_TXE));
            USART1->DR = (*ptr & 0x1FF);

            ++ptr;
            --size;
        }
    }
    else
    {
        /* write data directly */
        while (size)
        {
            //while (!(USART1->SR & USART_FLAG_TXE));
            //USART1->DR = (*ptr & 0x1FF);

            USART_SendData( USART1, *ptr );
            while( USART_GetFlagStatus( USART1, USART_FLAG_TC) == RESET);
            ++ptr;
            --size;
        }

    }
    return RT_EOK;
}

/***********************************************************
* Function:		gps_control
* Description:	控制模块
* Input:		rt_uint8_t cmd	命令类型
    void *arg       参数,依据cmd的不同，传递的数据格式不同
* Output:
* Return:
* Others:
***********************************************************/
#define RT_DEVICE_CTRL_BAUD	(RT_DEVICE_CTRL_SUSPEND+1)
static rt_err_t dev_vuart_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
    int i = *(int *)arg;

    RT_ASSERT(dev != RT_NULL);
    switch (cmd)
    {
    case RT_DEVICE_CTRL_SUSPEND:
        /* suspend device */
        dev_vuart.flag |= RT_DEVICE_FLAG_SUSPENDED;
        USART_Cmd(USART1, DISABLE);
        break;

    case RT_DEVICE_CTRL_RESUME:
        /* resume device */
        dev_vuart.flag &= ~RT_DEVICE_FLAG_SUSPENDED;
        USART_Cmd(USART1, ENABLE);
        break;
    case RT_DEVICE_CTRL_BAUD:
        uart1_baud(i);
        break;
    }

    return RT_EOK;

}


void rt_hw_usart_init()
{
    /*初始化接收缓冲区*/
    rt_ringbuffer_init(&rb_vuart, vuart_rxbuf, VUART_RXBUF_SIZE);

    dev_vuart.type	= RT_Device_Class_Char;
    dev_vuart.init	= dev_vuart_init;
    dev_vuart.open	= dev_vuart_open;
    dev_vuart.close	= dev_vuart_close;
    dev_vuart.read	= dev_vuart_read;
    dev_vuart.write	= dev_vuart_write;
    dev_vuart.control = dev_vuart_control;
    dev_vuart.rx_indicate = RT_NULL;
    dev_vuart.tx_complete = RT_NULL;

    rt_device_register( &dev_vuart, "vuart", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STREAM );
    rt_device_init( &dev_vuart );
}


