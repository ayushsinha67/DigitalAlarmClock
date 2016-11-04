#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

/************************************************************************
 *	INITIALIZE TIMER
 *  
 *  TIMER0 - CTC MODE, F_CPU: 16Mhz, PS: 64, Frequency: 1 KHz, Period: 1 ms  
 */

void Timer_Init(void)
{
    /* TIMER0 */
	TCCR0 |= (1<<WGM01) | (1<<CS01) | (1<<CS00);			/* CTC, PS: 64 */
    TIMSK |= (1<<OCIE0);									/* Enable Interrupt */
    OCR0 = 249;
}

