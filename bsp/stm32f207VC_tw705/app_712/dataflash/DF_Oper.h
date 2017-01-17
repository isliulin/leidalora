
//================================================================
/*         ��дAT45DB16������ͷ�ļ�
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



<һ>   ϵͳ���� �Լ� Ӧ�ò���  �г���¼��ص�ַ�洢����
*/


/*               Dataflash Page  �滮   ------->    Start          */

/*  0. Page 0~9     Producet Info */
#define DF_ProInfo_Page      0

/*  1. page 10 -903	  ISP	*/
#define ISP_StartArea                                 0x1000        // ��ʼ��ַ 
#define DF_APP1_PageNo		                          8             /*
start :   0x1000---- ��Boot �����Ӧ  8 page
size        60 sector     480 page

DF_APP_flah run PageNo:   50  ~ 903  page        */
/* 512K  -->1072 Page */


//   2.     config   information       904~1023 Page
#define    ConfigStart_offset                         1808 //904        //   Block   ��ʼλ��  Conifg  Struct Save      Sector 1 
#define    ConfigStart_BakSetting_offset              1824//912        //   Block   ��ʼλ?
#define    ConfigStart_Bak2Setting_offset             1840 //920       //   Block   ��ʼλ?

#define  DF_CycleAdd_Page                       928     // Block ��ʼ-- ��¼ѭ���洢��дƫ�Ƶ�ַ��page
#define  DF_PhotoAdd_Page                       936     // Block ��ʼ--��¼��Ƭ�洢��дƫ�Ƶ�ַ��page 


//��������� �� 1023  ( ��1024Page) PageSize            subPageSize   2047
#define       CycleStart_offset                       2048  // WP page                                 
//  WP25Q64   64Mbit = 4MByte     = 32768* WP_Page(256Bytes)
#define     LORA_Record_MAX      10100   // ���洢10100 ����¼ÿ����¼ 256Bytes     
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
