/*
 * ALARM
 * Oct 2016
 * Authors: Ayush
 */

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "lcd_hd44780.h"
#include "uart.h"
#include "i2c.h"
#include "rtc.h"

#define CHK(x,b) (x&b)
#define CLR(x,b) (x&=~b)
#define SET(x,b) (x|=b)
#define TOG(a,b) (a^=b)

/*DEFINES */
#define DDR_BUZZER			DDRC
#define PORT_BUZZER			PORTC
#define PIN_BUZZER			PINC
#define BUZZER				7
#define DDR_TOUCH			DDRA
#define PORT_TOUCH			PORTA
#define PIN_TOUCH			PINA
#define TOUCH				0

#define DDR_BUTTONS			DDRC
#define PORT_BUTTONS		PORTC
#define PIN_BUTTONS			PINC
#define UP_BUTTON			6
#define DOWN_BUTTON			5
#define SEL_BUTTON			4

/* ENUMERATIONS */
typedef enum 
{
	SHOW_TIME = 0,
	RING_ALARM,
	SET_ALARM,
	SET_TIME
	
} AlarmStatus;

typedef enum
{
	UP_BUT = 0,
	DOWN_BUT,
	SEL_BUT,
	NONE_BUT
} ButtonPressed;

/* VARIABLES */
volatile uint16_t RingCount = 0;
volatile uint16_t AlarmRingCounter = 0;
volatile uint16_t AlarmRingTime = 3 * 60 * 2; //  3 min
volatile rtc_t rtc;
volatile AlarmStatus Status;
volatile uint8_t disp_status = 1;
int alarm_onff = 1;
volatile int alarm_hour = 0;
volatile int alarm_min = 0;
volatile int alarm_ampm = 1;
int EEMEM alarm_onoff_eep = 1; 
int EEMEM alarm_hour_eep = 12;
int EEMEM alarm_min_eep = 5;
int EEMEM alarm_ampm_eep = 1;		// 0 -> AM, 1 -> PM

/* FUNCTION PROTOTYPES */
void display ( rtc_t *rtc );
void UART_TxRTC ( rtc_t *rtc  );
ButtonPressed GetButtonPressed(void);
void SetDisplay (void);
void SetAlarm (void);
void AlarmTimePrint( int s, int h, int m, int p );
void SetTime ( rtc_t *rtc );
void SetDateTime( rtc_t *rtc, uint8_t time[] );
void SetTimePrint( uint8_t time[] );
uint8_t ChkAlarmRing(rtc_t *rtc);
void Ports_Init(void);

/* -------------- MAIN ------------------- */
int main(void)
{
	ButtonPressed Button;	

    /* Initialize*/
	Ports_Init();												// Ports
	Timer_Init();												// Start Timer
	UART_Init();												// Set UART
	I2C_Init( TWI_MODE_MASTER, DS1307WriteMode);				// I2C
	RTC_Init();													// RTC
    lcd_init(LCD_DISP_ON);										// LCD
    lcd_clrscr();
	Status = SHOW_TIME;											// Set start status
	
	alarm_onff = eeprom_read_word(&alarm_onoff_eep);
	alarm_hour = eeprom_read_word(&alarm_hour_eep);
	alarm_min  = eeprom_read_word(&alarm_min_eep);
	alarm_ampm = eeprom_read_word(&alarm_ampm_eep);
	
    while(1){
		
		switch (Status)
		{	
			case SHOW_TIME:
				RTC_GetDateTime(&rtc);									// Update Time
				display(&rtc);
				if( ChkAlarmRing(&rtc) )
					Status = RING_ALARM;
				Button = GetButtonPressed();
				if( Button == SEL_BUT )						    // Set Time or Alarm
					SetDisplay();
				break;
				
			case RING_ALARM:
				sei();											// enable interrupts
				SET( PORT_BUZZER, 1<<BUZZER );
				if( CHK( PIN_TOUCH, 1<<TOUCH) ){
					Status = SHOW_TIME;
					CLR( PORT_BUZZER, 1<<BUZZER );
					lcd_clrscr();
					lcd_sets(3, 0, "Alarm Off");
					for( int i = 1; i<=6; i++)
						_delay_ms(250);
					cli();
					Ports_Init();
					lcd_clrscr();
				}				
				break;				
				
			case SET_TIME:
				SetTime(&rtc);
				break;
				
			case SET_ALARM:
				SetAlarm();
				break;
				
			default: 
				break;
				
		}				
    }

    return 0;
}

/* ------------ FUNCTIONS -------------------- */
/* Set Ports */
void Ports_Init(void)
{
	/* Initialize PORTS */
	SET( DDR_BUZZER, 1<<BUZZER);							// Set Buzzer
	CLR( DDR_TOUCH, 1<<TOUCH);								// Touch button input
	CLR( PORT_TOUCH, 1<<TOUCH);								// Pull Down
	
	CLR( DDR_BUTTONS, (1<<UP_BUTTON) | (1<<DOWN_BUTTON) | (1<<SEL_BUTTON) );
	SET( PORT_BUTTONS, (1<<UP_BUTTON) | (1<<DOWN_BUTTON) | (1<<SEL_BUTTON) ); // Pull Up
}

/* Set display */
void SetDisplay(void)
{
	int p = 0, exit = 0;
	ButtonPressed b = NONE_BUT;
	
	lcd_init(LCD_DISP_ON_CURSOR_BLINK);
	lcd_clrscr();
	lcd_sets(0, 0, "<Set Alarm");
	lcd_sets(0, 1, "<Set Time" );
	lcd_sets(11, 0, "<Exit" );
	
	while( exit == 0 ){
		
		b = GetButtonPressed();
		
		if( b == UP_BUT ){
			p++;
			if( p > 2 )
				p = 0;
		}	
		if ( b == DOWN_BUT ){
			p--;
			if ( p < 0 )
				p = 2;
		}
		if ( b == SEL_BUT ){
			exit = 1;
		}
		
		if( p == 0 )
			lcd_gotoxy(0, 0 );
		if (p == 1 )
			lcd_gotoxy(11, 0 );
		if (p == 2 )
			lcd_gotoxy(0, 1 );					
	}
	
	if ( p == 0 )
		Status = SET_ALARM;
	if ( p == 2 )
		Status = SET_TIME;
		
	lcd_clrscr();	
}

/* Set Alarm */
void SetAlarm( void )
{
	int cursor = 0, x, y, exit = 0;
	ButtonPressed b;
	
	lcd_init(LCD_DISP_ON_CURSOR_BLINK);
	lcd_clrscr();
	
	AlarmTimePrint( alarm_onff, alarm_hour, alarm_min, alarm_ampm );
	
	while( exit == 0 ){
		
		b = GetButtonPressed();
		
		if( b == UP_BUT ){
				
			if( cursor == 0 )
				TOG( alarm_onff, 0x01 );
			if( cursor == 1 ){
				alarm_hour++; 
				if( alarm_hour > 12 )
					alarm_hour = 0;
			}
			if( cursor == 2 ){
				alarm_min++;
				if( alarm_min > 59 )
					alarm_min = 0;
			}
			if( cursor == 3 ){
				TOG( alarm_ampm, 0x01 );
			}
			AlarmTimePrint( alarm_onff, alarm_hour, alarm_min, alarm_ampm );
		}
		
		if ( b == DOWN_BUT ){
		
			if( cursor == 0 )
				TOG( alarm_onff, 0x01 );
			if( cursor == 1 ){
				alarm_hour--;
				if( alarm_hour < 0 )
					alarm_hour = 12;
			}
			if( cursor == 2 ){
				alarm_min--;
				if( alarm_min < 0 )
					alarm_min = 59;
			}
			if( cursor == 3 ){
				TOG( alarm_ampm, 0x01 );
			}
			AlarmTimePrint( alarm_onff, alarm_hour, alarm_min, alarm_ampm );
			
		}
		if ( b == SEL_BUT ){
			
			if( cursor == 0 ){
				if( alarm_onff == 0 ){
					eeprom_write_word(&alarm_onoff_eep, alarm_onff);
					exit = 1;
					Status = SHOW_TIME;
					lcd_init(LCD_DISP_ON);
					lcd_sets( 2, 0, "Alarm Off");
					for(int i = 0; i<4; i++)
					_delay_ms(250);
				}
				else
					cursor = 1;	
			}
			else if( cursor == 1 )
				cursor++;
			else if( cursor == 2 )
				cursor++;
			else {
				/* Send to EEPROM */
				eeprom_write_word(&alarm_onoff_eep, alarm_onff);
				eeprom_write_word(&alarm_hour_eep, alarm_hour );
				eeprom_write_word(&alarm_min_eep, alarm_min );
				eeprom_write_word(&alarm_ampm_eep, alarm_ampm);
				exit = 1;
				Status = SHOW_TIME;
				lcd_init(LCD_DISP_ON);
				lcd_clrscr();
				lcd_sets( 2, 0, "Alarm Set");
				for(int i = 0; i<4; i++)
					_delay_ms(250);
			}
		}
		
		if( cursor == 0 ){ x=7, y=0; }
		if( cursor == 1 ){ x=0, y=1; }
		if( cursor == 2 ){ x=3, y=1; }
		if( cursor == 3 ){ x=6, y=1; }
		lcd_gotoxy(x,y);
	}	
}

/* alarm time print */
void AlarmTimePrint( int s, int h, int m, int p )
{
	lcd_sets(0, 0, "Alarm <");
	if( s == 0){
		lcd_puts("OFF ");
		lcd_sets(0, 1, "                  ");
	}
	else{
		lcd_puts("ON ");
		lcd_gotoxy(0,1);
		if( h < 10 )
			lcd_put_int(0);
		lcd_put_int(h);
		lcd_putc(':');
		if( m < 10 )
			lcd_put_int(0);
		lcd_put_int(m);
		lcd_putc(' ');
		if( p == 0 )
			lcd_puts("AM  ");
		else
			lcd_puts("PM  ");
	}
}

/* Set Time */
void SetTime ( rtc_t *rtc )
{
	uint8_t time[8], cursor = 0, x, y, exit = 0;
	
	time[0] = ( ( (rtc->hour&0x10) >> 4 ) * 10 ) + (rtc->hour&0x0F);
	time[1]  = ( ( (rtc->min&0x70) >> 4  ) * 10 ) + (rtc->min&0x0F);
	time[2] = ( ( rtc->hour & 0x20 ) >> 5 );
	time[3] = ( rtc->weekDay & 0x07 );
	time[4] = ( ( (rtc->date&0x30) >> 4 ) * 10 ) + (rtc->date&0x0F);
	time[5] = ( ( (rtc->month&0x10) >> 4 ) * 10 ) + (rtc->month&0x0F);
	time[6] = ( ( (rtc->year&0xF0) >> 4 ) * 10 ) + (rtc->year&0x0F);
	time[7] = 1; //time setting on off
	
	ButtonPressed b;
	
	lcd_init(LCD_DISP_ON_CURSOR_BLINK);
	lcd_clrscr();
	
	SetTimePrint(&time);
	
	while( exit == 0 ){
		
		b = GetButtonPressed();
		
		if( b == UP_BUT ){
			
			switch( cursor ){
				
				case 0: TOG( time[7], 0x01 ); 
						break;
				case 1: time[0]++;
						if( time[0] > 12 )
							time[0] = 0; 
						break;
				case 2: time[1]++;
						if( time[1] > 59 )
							time[1] = 0;
						break;
				case 3: TOG( time[2], 0x01 );
						break;
				case 4: time[3]++;
						if( time[3] > 7 )
							time[3] = 1;
						break;
				case 5: time[4]++;
						if( time[4] > 31 )
							time[4] = 1;
						break;
				case 6: time[5]++;
						if( time[5] > 12 )
							time[5] = 1;
						break;
				case 7: time[6]++;
						if( time[6] > 99 )
							time[6] = 0;
						break;
				default: break;
			}
		
			SetTimePrint(&time);
		}
		
		if ( b == DOWN_BUT ){
			
			switch( cursor ){
				
				case 0: TOG( time[7], 0x01 );
						break;
				case 1: time[0]--;
						if( time[0] == 255 )
							time[0] = 12;
						break;
				case 2: time[1]--;
						if( time[1] == 255 )
							time[1] = 59;
						break;
				case 3: TOG( time[2], 0x01 );
						break;
				case 4: time[3]--;
						if( time[3] < 1 )
							time[3] = 7;
						break;
				case 5: time[4]--;
						if( time[4] < 1 )
							time[4] = 31;
						break;
				case 6: time[5]--;
						if( time[5] < 1 )
							time[5] = 12;
						break;
				case 7: time[6]--;
						if( time[6] == 255 )
							time[6] = 99;
						break;
				default: break;
			}
			
			SetTimePrint(&time);			
			
		}
		if ( b == SEL_BUT ){
			
			if( cursor == 0 ){
				if( time[7] == 0 ){
					exit = 1;
					Status = SHOW_TIME;
					lcd_init(LCD_DISP_ON);
					lcd_sets( 2, 0, "Time Not Set");
					for(int i = 0; i<4; i++)
					_delay_ms(250);
				}
				else
					cursor = 1;
			}
			else if( (cursor >= 1) && (cursor <= 6) )
				cursor++;
			else {
				SetDateTime(&rtc, &time); // Set to RTC
				exit = 1;
				Status = SHOW_TIME;
				lcd_init(LCD_DISP_ON);
				lcd_clrscr();
				lcd_sets( 2, 0, "Time Set");
				for(int i = 0; i<4; i++)
					_delay_ms(250);
			}
		}
	
	switch (cursor){
		case 0: x=0, y=0; break;
		case 1: x=5, y=0;  break;
		case 2: x=8, y=0;  break;
		case 3: x=11, y=0;  break;
		case 4: x=9, y=1;  break;
		case 5: x=0, y=1;  break;
		case 6: x=3, y=1;  break;
		case 7: x=6, y=1;
		default: break;
	}	
	
	lcd_gotoxy(x,y);
	}	
}

/* Set Time Print */
void SetTimePrint( uint8_t time[] )
{
	lcd_gotoxy(0,0);
	if( time[7] == 0){
		lcd_clrscr();
		lcd_puts("<NO ");
	}
	else{
		lcd_puts("<Set ");
		lcd_gotoxy(5,0);
		if( time[0] < 10 )
			lcd_put_int(0);
		lcd_put_int(time[0]);
		lcd_putc(':');
		if( time[1] < 10 )
			lcd_put_int(0);
		lcd_put_int(time[1]);
		lcd_putc(' ');
		if( time[2] == 0 )
			lcd_puts("AM  ");
		else
			lcd_puts("PM  ");
		
		lcd_gotoxy(0, 1);
		if( time[4] < 10 )
			lcd_put_int(0);
		lcd_put_int(time[4]);
		lcd_putc('/');
		if( time[5] < 10 )
			lcd_put_int(0);
		lcd_put_int(time[5]);
		lcd_putc('/');
		if( time[6] < 10 )
			lcd_put_int(0);
		lcd_put_int(time[6]); 
		lcd_putc(' ');
		
		char *s;
		switch(time[3]){
			case 1: s = "Sun"; break;
			case 2: s = "Mon"; break;
			case 3: s = "Tue"; break;
			case 4: s = "Wed"; break;
			case 5: s = "Thu"; break;
			case 6: s = "Fri"; break;
			case 7: s = "Sat"; break;
			default: break;
		}
		lcd_puts(s);
	}
}

/* Check Alarm Ring Time */
uint8_t ChkAlarmRing( rtc_t *rtc )
{	
	uint8_t hour = ( ( (rtc->hour&0x10) >> 4 ) * 10 ) + (rtc->hour&0x0F);
	uint8_t min  = ( ( (rtc->min&0x70) >> 4  ) * 10 ) + (rtc->min&0x0F);
	uint8_t sec  = ( ( (rtc->sec&0x70) >> 4  ) * 10 ) + (rtc->sec&0x0F);
	uint8_t ampm = ( ( rtc->hour & 0x20 ) >> 5 ); 
	
	if( (hour == alarm_hour) && (min == alarm_min) && (ampm == alarm_ampm) && (sec == 0) && (alarm_onff == 1) )
		return 1;
	else
		return 0;
}

/* Get Button Pressed */
ButtonPressed GetButtonPressed(void)
{
	if( !CHK( PIN_BUTTONS, 1<<UP_BUTTON) ){
		_delay_ms(200);
		return UP_BUT;	
	}
	else if ( !CHK( PIN_BUTTONS, 1<<DOWN_BUTTON) ){
		_delay_ms(200);
		return DOWN_BUT;
	}
	else if ( !CHK( PIN_BUTTONS, 1<<SEL_BUTTON) ){
		_delay_ms(200);
		return SEL_BUT;
	}
	else
		return NONE_BUT;
}

/* Set Date and Time */
void SetDateTime( rtc_t *rtc, uint8_t time[] )
{
	rtc->sec = 0x00;
	rtc->min = ( time[1]%10 ) | ( (time[1]/10) << 4 );
	rtc->hour = ( ( time[0]%10 ) | ( (time[0]/10) << 4 ) ) | 0x40 | (time[2]<<5);
	rtc->weekDay = time[3];
	rtc->date = ( ( time[4]%10 ) | ( (time[4]/10) << 4 ) ); 
	rtc->month = ( ( time[5]%10 ) | ( (time[5]/10) << 4 ) );
	rtc->year = ( ( time[6]%10 ) | ( (time[6]/10) << 4 ) );
	
	UART_TxRTC(rtc);
	RTC_SetDateTime(rtc);
}

/* Display RTC on LCD */
void display ( rtc_t *rtc )
{
	lcd_gotoxy( 0, 0 );
	
	if ( (rtc->hour & 0x40) == 0 )				// 24 hour mode
		lcd_put_int( (rtc->hour&0x30) >> 4 ) ;
	else
		lcd_put_int( (rtc->hour&0x10) >> 4 ) ;
		
	lcd_put_int(rtc->hour&0x0F);
	lcd_putc(':');
	lcd_put_int( (rtc->min&0x70) >> 4 );
	lcd_put_int( (rtc->min&0x0F) );
	lcd_putc(':');
	lcd_put_int( (rtc->sec&0x70) >> 4 );
	lcd_put_int( (rtc->sec&0x0F) );
	
	if ( rtc->hour & 0x40 ){				// 12 hour mode
		if( rtc->hour & 0x20 )
			lcd_puts(" PM    ");
		else
			lcd_puts(" AM    ");	
	}				
	
	lcd_gotoxy( 0, 1 );
	lcd_put_int( (rtc->date&0x30) >> 4 );
	lcd_put_int( (rtc->date&0x0F) );
	lcd_putc('/');
	lcd_put_int( (rtc->month&0x10) >> 4 );
	lcd_put_int( rtc->month&0x0F );
	lcd_putc('/');
	lcd_put_int( (rtc->year&0xF0) >> 4 );
	lcd_put_int( rtc->year&0x0F );
	lcd_putc(' ');
	
	char *s;
	switch(rtc->weekDay){
		case 1: s = "Sun"; break;
		case 2: s = "Mon"; break;
		case 3: s = "Tue"; break;
		case 4: s = "Wed"; break;
		case 5: s = "Thu"; break;
		case 6: s = "Fri"; break;
		case 7: s = "Sat"; break;
		default: break;
	}
	lcd_puts(s);

}

/* Tx RTC on Terminal */
void UART_TxRTC( rtc_t *rtc )
{
	if ( ( rtc->hour & 0x40 ) == 0 )				// 24 hour mode
		UART_TxInt( (rtc->hour&0x30) >> 4 ) ;
	else
		UART_TxInt( (rtc->hour&0x10) >> 4 ) ;
	
	UART_TxInt(rtc->hour&0x0F);
	UART_TxChar(':');
	UART_TxInt( (rtc->min&0x70) >> 4 );
	UART_TxInt( (rtc->min&0x0F) );
	UART_TxChar(':');
	UART_TxInt( (rtc->sec&0x70) >> 4 );
	UART_TxInt( (rtc->sec&0x0F) );
	
	if ( rtc->hour & 0x40 ){				// 12 hour mode
		if( rtc->hour & 0x20 )
			UART_TxStr(" PM");
		else
			UART_TxStr(" AM");
	}

	UART_TxChar('\n');
	UART_TxInt( (rtc->date&0x30) >> 4 );
	UART_TxInt( (rtc->date&0x0F) );
	UART_TxChar('/');
	UART_TxInt( (rtc->month&0x10) >> 4 );
	UART_TxInt( rtc->month&0x0F );
	UART_TxChar('/');
	UART_TxInt( (rtc->year&0xF0) >> 4 );
	UART_TxInt( rtc->year&0x0F );
	UART_TxChar(' ');

	char *s;
	switch(rtc->weekDay){
		case 1: s = "Sunday"; break;
		case 2: s = "Monday"; break;
		case 3: s = "Tuesday"; break;
		case 4: s = "Wednesday"; break;
		case 5: s = "Thursday"; break;
		case 6: s = "Friday"; break;
		case 7: s = "Saturday"; break;
		default: break;
	}
	UART_TxStr(s);
	UART_TxChar('\n');
}

/*--------------------------------------------------------------
                 ISR DEFINITIONS
---------------------------------------------------------------*/
ISR( TIMER0_COMP_vect )
{
   if( Status == RING_ALARM ){
        RingCount++;
		if( RingCount >= 500 ){
			RingCount = 0;
			AlarmRingCounter++;
			if( disp_status == 1 ){
				lcd_clrscr();
				lcd_sets(3, 0, "WAKE UP!");	
			}
			else
				lcd_clrscr();
			TOG( disp_status, 0x01 );
		}
	    if( AlarmRingCounter >= AlarmRingTime){
			AlarmRingCounter = 0;
			RingCount = 0;
			Status = SHOW_TIME;
			CLR( PORT_BUZZER, 1<<BUZZER );
			lcd_clrscr();
			lcd_sets(3, 0, "Alarm Off");
			for( int i = 1; i<=6; i++)
			_delay_ms(250);
			cli();	
	  }			
    }
}