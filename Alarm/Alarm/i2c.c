#include <avr/io.h>
#include <util/twi.h>
#include "i2c.h"

/************************************************************************
 *	TWI INITIALIZATION
 *
 * TWI CLOCK: 100 KHz
 * PRESCALAR: 1 
 * F_SCL = F_CPU / ( 16 + ( 2 * TWBR * 4^TWPS ) )
 * 
 */
void I2C_Init ( uint8_t mode, uint8_t addr )
{
	//TWAR = ( addr << 1 );										/* Set device address */
	
	switch ( mode ) {
		
		case TWI_MODE_MASTER :	TWSR = 0x00;					/* No prescalar, Set Bit Rate */
								TWBR = ( ( F_CPU / TWI_CLK ) - 16 ) / 2 ;
								break;
		
		case TWI_MODE_SLAVE :	TWCR = ( 1 << TWEN ) | ( 1 << TWEA );
								break;
				
		default				:   break;
	}
	
#if ( TWI_INT_ENABLE == 1 )										/* Enable TWI Interrupt */
	TWCR |= ( 1 << TWIE );
#endif
	
#if ( TWI_GCE == 1 )											/* Enable General Call receive */
	TWAR |= ( 1 << TWGCE );
#endif
}

/************************************************************************
 *	TWI START
 */
void I2C_Start ( void )
{
	TWCR = ( 1 << TWEN ) | ( 1 << TWINT ) | ( 1 << TWSTA );		/* Send START */
	while ( !( TWCR & ( 1 << TWINT ) ) );						/* Wait for START to transmit */
}

/************************************************************************
 *	TWI STOP
 */
void I2C_Stop ( void )
{
	TWCR = ( 1 << TWEN ) | ( 1 << TWINT ) | ( 1 << TWSTO );		/* Send STOP */
	while ( TWCR & ( 1 << TWSTO ) );							/* Wait for STOP to transmit */
}

/************************************************************************
 *	TWI WRITE
 */
void I2C_Write ( uint8_t data)
{
	TWDR = data;												/* Write to data register */
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}

/************************************************************************
 *	TWI READ
 */
uint8_t I2C_Read ( uint8_t ack )
{
	if( ack == 1 )
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	else
		TWCR = (1<<TWINT)|(1<<TWEN);
	
	while ( !( TWCR & ( 1 << TWINT ) ) );						/* Wait for data to be read */
	return TWDR;
}



