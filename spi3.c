/*
 * spi3.c
 *

 i got this off the off the AVR site's examples, 
but needed to make a few changes.

setup:
 You must set pin #define correctly for given hardware or this won't work.
 Defines are in the spi3.h file
 For uno: MISO is PB4, MOSI is PB4, SCK is PB5
 SPI_CS can be whatever pin you want (set as output).
 
 
use:

 make sure to SELECT() before i/o and DESELECT() when done with i/o
 
 use spi_mode(x) to set SPI mode 1,2,3, any other value is mode 0.
 
 AVR chip is always master while using this library.
 
 also:
 your c file with main{} will need to #include <avr/io.h> or this won't compile.
 */ 


#include <avr/io.h>
#include "spi3.h"
#include <stdint.h>



void init_spi_master(void)
{
	/* SPI pins */
	SPIDDR |= (SPI_MOSI | SPI_CS | SPI_SCK); /* Set outputs */
	SPIPORT |= SPI_MISO; /* Pull up on SPI_MISO within AVR chip */

	/* Master Mode, 100-400kHz clk  */
	/* Setup Peripheral Clock Prescaler */
	  // enable SPI, set as master, and clock to fosc/128
	 // SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
	/* SPI Master mode */
	SPCR |= (1 << MSTR);
	/* Set clock speed (16Mhz osc 0,1 = 1Mhz */
	SPCR &= ~(1 << SPR0);
	SPCR |= (1 << SPR1);
	
	/* SPI Enable */
	SPCR |= (1 << SPE);
}


void spi_mode(char mode)
{
if (mode == 1)
	{
	SPCR &= ~(1 << CPOL);
	SPCR |= (1 << CPHA);
	}
if (mode == 2)
	{
	SPCR |= (1 << CPOL);
	SPCR &= ~(1 << CPHA);
	}
if (mode == 3)
	{
	SPCR |= (1 << CPOL);
	SPCR |= (1 << CPHA);
	}
else
		{
    	SPCR &= ~(1 << CPOL);
    	SPCR &= ~(1 << CPHA);	
		}
	
}

void SPI_SendByte(uint8_t data)
 {
	 SPDR = data;
	 while (!(SPSR & (1<<SPIF)));
	 
 }

void SPI_Send2Bytes(uint16_t data)
{
	SPDR = (data & 0xFF00) << 8 ;
	while (!(SPSR & (1<<SPIF)));
    
	SPDR = data & 0x00FF ;
	while (!(SPSR & (1<<SPIF)));
	
}