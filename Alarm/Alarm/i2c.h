#ifndef I2C_H_
#define I2C_H_

/************************************************************************
 *	DEFINES
 */
#ifndef F_CPU
#define F_CPU					16000000UL					/* CPU clock frequency */
#endif

#define TWI_MODE_MASTER			1							/* MASTER mode */
#define TWI_MODE_SLAVE			0							/* SLAVE mode */
#define TWI_CLK					100000UL					/* TWI clock frequency */
#define TWI_GCE					0							/* 0: Disabled, 1: Enable TWI General Call receive, Address: 0x00 */
#define TWI_INT_ENABLE			0							/* 0: Disabled, 1: Enable TWI Interrupt mode */
#define TWI_MAX_ERROR_COUNT		500							/* Error counter for NO interrupt mode */


/************************************************************************
 *	FUNCTION PROTOTYPES
 */
void		I2C_Init		( uint8_t mode, uint8_t addr );										/* TWI Initialization */
void		I2C_Start		( void );															/* TWI Start */
void		I2C_Stop		( void );															/* TWI Stop */
void		I2C_Write		( uint8_t data);													/* TWI Write */
uint8_t		I2C_Read		( uint8_t ack );													/* TWI Read */


#endif /* I2C_H_ */