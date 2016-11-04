/*
 * rtc.c
 *
 * Created: 31-10-2016 15:12:37
 *  Author: Ayush
 */ 

#include <avr/io.h>
#include "i2c.h"
#include "rtc.h"

/* RTC Initialize */
void RTC_Init(void)
{
	I2C_Start();							// Start I2C communication
	I2C_Write(DS1307WriteMode);				// Connect to DS1307 by sending its ID on I2c Bus
	I2C_Write(DS1307_CONTROL);				// Select the Ds1307 ControlRegister to configure Ds1307
	I2C_Write(0x00);						// Write 0x00 to Control register to disable SQW-Out
	I2C_Stop();								// Stop I2C communication after initializing DS1307
}

/* Get Date and Time */
void RTC_GetDateTime(rtc_t *rtc)
{
	I2C_Start(); 							// Start I2C communication
	I2C_Write(DS1307WriteMode);				// connect to DS1307 by sending its ID on I2c Bus
	I2C_Write(DS1307_SEC_ADD);				// Request Sec RAM address at 00H
	I2C_Stop();								// Stop I2C communication after selecting Sec Register
	
	I2C_Start();	
	I2C_Write(DS1307ReadMode);				// connect to DS1307(Read mode) by sending its ID
	
	rtc->sec = I2C_Read(1);					// read second and return Positive ACK
	rtc->min = I2C_Read(1);					// read minute and return Positive ACK
	rtc->hour= I2C_Read(1);					// read hour and return Negative/No ACK
	rtc->weekDay = I2C_Read(1);				// read weekDay and return Positive ACK
	rtc->date= I2C_Read(1);					// read Date and return Positive ACK
	rtc->month=I2C_Read(1);					// read Month and return Positive ACK
	rtc->year =I2C_Read(0);					// read Year and return Negative/No ACK
	I2C_Stop();								// Stop I2C communication after reading the Date
}

/* Set Date and Time */
void RTC_SetDateTime(rtc_t *rtc)
{
	I2C_Start();							// Start I2C communication
	
	I2C_Write(DS1307WriteMode);				// Connect to DS1307 by sending its ID on I2c Bus
	I2C_Write(DS1307_SEC_ADD);				// Request sec RAM address at 00H
	
	I2C_Write(rtc->sec);                    // Write sec from RAM address 00H
	I2C_Write(rtc->min);                    // Write min from RAM address 01H
	I2C_Write(rtc->hour);                   // Write hour from RAM address 02H
	I2C_Write(rtc->weekDay);                // Write weekDay on RAM address 03H
	I2C_Write(rtc->date);                   // Write date on RAM address 04H
	I2C_Write(rtc->month);                  // Write month on RAM address 05H
	I2C_Write(rtc->year);                   // Write year on RAM address 06h
	
	I2C_Stop();                             // Stop I2C communication after Setting the Date
}

