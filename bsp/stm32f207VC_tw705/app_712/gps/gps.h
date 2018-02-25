#ifndef _GPS_H_
#define _GPS_H_

#define  RTC_8564


#define  RTC_8546_INT_PORT     GPIOB
#define  RTC_8546_INT_PIN      GPIO_Pin_5


#define I2C_TIMEOUT              0x1000

#define RTC_WR			0xA2
#define RTC_RE			0xA3

#define	reg_ctr1			0x00
#define	reg_ctr2			0x01
#define reg_sec				0x02
#define reg_min				0x03
#define	reg_hour			0x04
#define	reg_day				0x05
#define reg_week			0x06
#define reg_month			0x07
#define	reg_year			0x08
#define	reg_alarm_min		0x09
#define reg_alarm_hour		0x0A
#define reg_alarm_day		0x0B
#define reg_alarm_week		0x0C

#define reg_clk				0x0D
#define reg_timer_ctr		0x0E
#define reg_timer			0x0F

typedef struct RTC_TIME_
{
    uint8_t     BCD_6_Bytes[6]; 
	uint8_t		year;
	uint8_t		month;
	uint8_t		day;
	uint8_t		hour;
	uint8_t     min;
	uint8_t		sec;
	uint8_t		week;
}RTCTIME;
extern RTCTIME rtc_current;
extern u8	 Get_RTC_enableFlag;  

extern void RTC_8564_IIC_INIT(void);
extern void RTC8564_Init(void);
extern void RTC8564_Get(void);
extern void RTC8564_Set(RTCTIME time);

#endif

