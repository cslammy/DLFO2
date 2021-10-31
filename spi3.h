/*
 * spi.h
 *

 i got this off the off the AVR site's examples, 
but needed to make a few changes.

setup:
 You must set pin #define correctly for given hardware or this won't work.
 For uno: MISO is PB4, MOSI is PB3, SCK is PB5
 SPI_CS can be whatever pin you want (set as output).
 
 
use:

 make sure to SELECT() before i/o and DESELECT() when done with i/o
 
 AVR chip is always master while using this library.
 
 also:
 your c file with main{} will need to #include <avr/io.h> or this won't compile.
 */ 


#ifndef SPI_H_
#define SPI_H_
#include <avr/io.h>
#define SPIPORT			PORTB
#define SPIDDR			DDRB
#define SPI_SCK			(1 << 5) /* UNO 13 */
#define SPI_MISO		(1 << 4) /* UNO 12*/
#define SPI_MOSI		(1 << 3) /* UNO 11*/
#define SPI_CS			(1 << 2) /* UNO 10 */

/* CS controls */
#define SELECT()		SPIPORT &= (~SPI_CS)	/* CS = L */
#define	DESELECT()		SPIPORT |= SPI_CS		/* CS = H */
#define SELECTING		!(SPIPORT & SPI_CS)

/* declarations */
void init_spi_master(void);

void spi_mode(char mode);

void SPI_SendByte(uint8_t data);

void SPI_Send2Bytes(uint16_t data);

#endif /* SPI_H_ */