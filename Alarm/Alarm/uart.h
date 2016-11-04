#ifndef UART_H_
#define UART_H_

/************************************************************************
 *	DEFINES
 */
#define F_CPU			16000000UL
#define UART_BAUDRATE	9600UL
#define BAUD_PRESCALE	( ( ( F_CPU / ( UART_BAUDRATE * 16UL ) ) ) - 1 )
#define	UART_RX_INT		0
#define UART_TX_INT		0

/************************************************************************
 *	FUNCTION PROTOTYPES
 */
void UART_Init			( void );
char UART_RxChar		( void );
void UART_TxChar		( const char data );
void UART_TxStr			( const char *data );
void UART_TxStr_p		( const char *data );
void UART_TxNibble		( const unsigned char data );
void UART_TxHex			( const unsigned char data );
void UART_TxInt			( const int val );

#endif /* UART_H_ */

