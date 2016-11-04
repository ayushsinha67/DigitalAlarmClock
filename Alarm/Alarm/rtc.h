/*
 * rtc.h
 *
 * Created: 31-10-2016 15:12:46
 *  Author: Ayush
 */ 


#ifndef RTC_H_
#define RTC_H_

/************************************************************************
 *	DEFINES
 */
#define DS1307ReadMode		0xD1  // DS1307 ID
#define DS1307WriteMode		0xD0  // DS1307 ID
#define DS1307_SEC_ADD		0x00
#define DS1307_MIN_ADD		0x01
#define DS1307_HOUR_ADD		0x02
#define DS1307_DAY_ADD		0x03
#define DS1307_DATE_ADD		0x04
#define DS1307_MONTH_ADD	0x05
#define DS1307_YEAR_ADD		0x06
#define DS1307_CONTROL		0x07

/************************************************************************
 *	DATA TYPES
 */
typedef struct
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t weekDay;
	uint8_t date;
	uint8_t month;
	uint8_t year;
}rtc_t;

/************************************************************************
 *	FUNCTIONS
 */
void RTC_Init(void);
void RTC_GetDateTime(rtc_t *rtc);
void RTC_SetDateTime(rtc_t *rtc);

#endif /* RTC_H_ */