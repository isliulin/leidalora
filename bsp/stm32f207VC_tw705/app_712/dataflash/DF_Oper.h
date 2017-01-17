
//================================================================
/*         读写AT45DB16函数的头文件
MSP430 IAR application builder : 2007-04-15 9:00:00
Target : MSP430F149
Crystal: 3.6864Mhz
*/
//================================================================
//#include "common.h"
//#include "71x_type.h"

#ifndef _H_AT45
#define _H_AT45

#define    PageSIZE             512
#define    WinBond_PageSIZE     256
#define  DFBakSize   150//50 

//================================================================
/*
  Flash Chip : SSST25VF032B-50-4I-S2AF
  ChipSize       : 4MBytes       PageSize(vitual): 512Bytes  SectorSize:4K<=>8 Pages    Chip: 1024Sectors<=>8192Pages

  Regulation :



<一>   系统参数 以及 应用参数  行车记录相关地址存储部分
*/


/*               Dataflash Page  规划   ------->    Start          */

/*  0. Page 0~9     Producet Info */
#define DF_ProInfo_Page      0

/*  1. page 10 -903	  ISP	*/
#define ISP_StartArea                                 0x1000        // 起始地址 
#define DF_APP1_PageNo		                          8             /*
start :   0x1000---- 和Boot 程序对应  8 page
size        60 sector     480 page

DF_APP_flah run PageNo:   50  ~ 903  page        */
/* 512K  -->1072 Page */


//   2.     config   information       904~1023 Page
#define    ConfigStart_offset                         1808 //904        //   Block   起始位置  Conifg  Struct Save      Sector 1 
#define    ConfigStart_BakSetting_offset              1824//912        //   Block   起始位?
#define    ConfigStart_Bak2Setting_offset             1840 //920       //   Block   起始位?

#define  DF_CycleAdd_Page                       928     // Block 起始-- 记录循环存储读写偏移地址的page
#define  DF_PhotoAdd_Page                       936     // Block 起始--记录照片存储读写偏移地址的page 


//该区域结束 至 1023  ( 共1024Page) PageSize            subPageSize   2047
#define       CycleStart_offset                       2048  // WP page                                 
//  WP25Q64   64Mbit = 4MByte     = 32768* WP_Page(256Bytes)
#define     LORA_Record_MAX      10100   // 最多存储10100 条记录每条记录 256Bytes     
#define     LORA_LOG_OUT_MAX     10000




//-------------------------------------------------------


extern  u8   DF_initOver;    //     Dataflash  Lock


extern void DF_delay_us(u16 j);
extern void DF_delay_ms(u16 j); 
extern void DF_Erase(void);
extern void DF_init(void);
extern void DF_WP_pageWrite(u16 page_num, u8 *p, u16 length);
extern void DF_WP_pageRead(u16 page_num, u8 *p, u16 length);
extern void DF_ReadFlash(u16 page_counter, u16 page_offset, u8 *p, u16 length);


#endif
