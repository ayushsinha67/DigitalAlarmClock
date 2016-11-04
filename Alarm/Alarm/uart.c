#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "uart.h"

/************************************************************************
 *  INITIALIZE UART
 */
void UART_Init ( void )
{
    UCSRB |= ( 1 << RXEN ) | ( 1 << TXEN );						/* Turn on the transmission and reception circuitry */
    UCSRC |= ( 1 << URSEL ) | ( 1 << UCSZ0 ) | ( 1 << UCSZ1 );	/* Use 8-bit character sizes */
    UBRRL = BAUD_PRESCALE;										/* Lower byte of baud prescalar */
    UBRRH = ( BAUD_PRESCALE >> 8 );								/* Higher byte of baud prescalar */

#if ( UART_RX_INT == 1 )										/* RX Interrupt */
	UCSRB |= ( 1 << RXCIE );	
#endif

#if ( UART_TX_INT == 1 )										/* TX Interrupt */
	UCSRB |= ( 1 << TXCIE );	
#endif						
}

/************************************************************************
 *	RECEIVE CHARACTER
 */
char UART_RxChar ( void )
{
    while ( ( UCSRA & ( 1 << RXC ) ) == 0 );					/* Wait till Rx complete */
    char data = UDR;
    return data;
}

/************************************************************************
 *	TRANSMIT CHARACTER
 */
void UART_TxChar( const char data )
{
	while ( ( UCSRA & ( 1 << UDRE ) ) == 0);					/* Wait till data register is empty */
	UDR = data;
}

/************************************************************************
 *	TRANSMIT STRING
 */
void UART_TxStr ( const char *data )
{
	while ( *data != '\0' )										/* Transmit till termination */ 
		UART_TxChar( *data++ );  
}

/************************************************************************
 *	TRANSMIT STRING FROM PROGRAM MEMORY
 */
void UART_TxStr_p ( const char *data )
{
    while ( pgm_read_byte ( data ) != '\0' )					/* Transmit till termination */
		UART_TxChar( pgm_read_byte ( data++ ) ); 
}

/************************************************************************
 *	TRANSMIT LOWER NIBBLE OF BYTE AS HEX
 */
void UART_TxNibble ( const unsigned char data )
{
    unsigned char  c = ( data & 0x0F );							/* Extract lower nibble */
    
	if ( c > 9 )								
		c = ( c - 10 ) + 'A';									/* ASCII values greater than 9 */
    else 
		c += '0';												/* ASCII values less than 10 */
		
    UART_TxChar(c);												/* Transmit nibble */
} 

/************************************************************************
 *	TRANSMIT BYTE AS HEX
 */
void UART_TxHex ( const unsigned char data )
{
    UART_TxNibble( data >> 4 );									/* Transmit upper nibble */
	UART_TxNibble( data );										/* Transmit lower nibble */
} 

/************************************************************************
 *	TRANSMIT INTEGER AS ASCII 
 */
void UART_TxInt ( const int val )
{
    char buffer[ ( sizeof(int) * 8 ) + 1 ];						/* Create buffer */
	itoa( val, buffer, 10 );									/* Convert integer to string */
    UART_TxStr( buffer );										/* Transmit string */
}