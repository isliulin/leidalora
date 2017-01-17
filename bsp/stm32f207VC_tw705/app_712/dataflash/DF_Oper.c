/*****************************************************************
 MSP430 AT45DB041B 函数
*****************************************************************/
/*
#include <string.h>
*/
#include "App_moduleConfig.h"
#include "DF_Oper.h"


u8   DF_initOver = 0;  //Dataflash  Lock



void DF_init(void)
{
    u32 device_id = 0;

    SST25V_Init();

    SST25V_CS_LOW();
    SPI_Flash_SendByte(WriteDisable);
    SST25V_CS_HIGH();

    SST25V_CS_LOW();

    //-----erase chip-------------------
    //	DF_delay_ms(350);
    //--------------------------------

    SPI_Flash_SendByte( ReadJedec_ID  );
    device_id  = SPI_Flash_SendByte(0xFF) << 16;
    device_id = device_id | SPI_Flash_SendByte(0xFF) << 8;
    device_id = device_id | SPI_Flash_SendByte(0xFF);
    SST25V_CS_HIGH();

    //if(device_id == 0xBF254A)//SST25VF032B = 0xBF254A,
    {
        SST25V_CS_LOW();
        SPI_Flash_SendByte( DBSY );
        SST25V_CS_HIGH();

        SST25V_CS_LOW();
        SPI_Flash_SendByte( EnableWriteStatusRegister );
        SST25V_CS_HIGH();

        SST25V_CS_LOW();
        SPI_Flash_SendByte( WriteStatusRegister );
        SPI_Flash_SendByte( 0 );
        SST25V_CS_HIGH();
    }

    delay_ms(700);

}
void DF_delay_us(u16 j)
{
    u8 i;
    while(j--)
    {
        i = 3;
        while(i--);
    }
}

void DF_delay_ms(u16 j)
{
    while(j--)
    {
        DF_delay_us(2000);// 1000
    }

}


void DF_ReadFlash(u16 page_counter, u16 page_offset, u8 *p, u16 length)
{
    u16 i = 0;
    u32 ReadAddr = ((u32)page_counter * PageSIZE) + (u32)page_offset;


    //----  Addr +  content 内容对比-----
    SST25V_BufferRead(p, ReadAddr, length);
    DF_delay_ms(5);

}

void DF_WP_pageWrite(u16 page_num, u8 *p, u16 length)
{
   u32  Addr_24bit=page_num*WinBond_PageSIZE;

   WP64_Chip_write_page(p,length,Addr_24bit);
   delay_us(80);
}

void DF_WP_pageRead(u16 page_num, u8 *p, u16 length)
{
   u32  Addr_24bit=page_num*WinBond_PageSIZE;

   WP64_Chip_read_page(p,length,Addr_24bit);
   delay_us(80);
}

void DF_Erase(void)
{
    u16 i = 0;
    /*
         1. 从0x1000    4K  开始擦除28K     7 sector
         2.擦除56个扇区   7 个32k block
      */

    DF_TAKE;
    rt_kprintf("\r\n  ISP erase start\r\n");
    //  -----erase        28 K   area  -------------
    for(i = 0; i < 7; i++) // 7  secotor
    {
        WatchDog_Feed();
        SST25V_SectorErase_4KByte(ISP_StartArea + i * 0x1000);
        DF_delay_ms(220);
        WatchDog_Feed();
    }
    //----------- erase  32k
    for(i = 0; i < 7; i++) // 56sector
    {
        WatchDog_Feed();
        SST25V_BlockErase_32KByte(0x8000 + i * 0x8000);
        DF_delay_ms(800);
        WatchDog_Feed();
    }
    DF_RELEASE;
    rt_kprintf("\r\n  ISP erase OK DFReady\r\n");
}
//FINSH_FUNCTION_EXPORT(DF_Erase, DF_Erase);

void DF_WriteFlashDirect(u16 page_counter, u16 page_offset, u8 *p, u16 length) //512bytes 直接存储
{
    u16 i = 0;

    for(i = 0; i < length; i++)
    {
        SST25V_ByteWrite(*p, (u32)page_counter * PageSIZE + (u32)(page_offset + i));
        p++;
    }
    DF_delay_ms(5);
}



