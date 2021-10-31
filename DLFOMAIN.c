#define F_CPU 16000000UL
#define BAUD 9600
#define BAUD_TOL 2
#include <avr/io.h>
#include "spi3.h"
#include <stdint.h>
#include <stddef.h>
#include <util/delay.h>
#include "ADClib.h"
#include "stdio_setup.h"
#include <stdio.h>
#include <math.h>

void SPI_TransferTx16(uint8_t a, uint8_t b);
uint8_t gethexcount(uint32_t n);

uint8_t CV = 0;


int main(void)
{
	//set up stdio_setup
	UartInit();
	
	//initialize SPI
	init_spi_master();
	_delay_ms(100);
	spi_mode(2);
	_delay_ms(15);
	DESELECT();
	SELECT();

	SPI_TransferTx16(33,0); //control 16 bits w reset 2 byte freq write


	
	while (1)
	{

		
		CV = analogRead10bit();
		// do some damn thing


	}
}



uint8_t gethexcount(uint32_t n)
{
	//counts # of hex digits for incoming # n
	uint8_t count;

	count = 0;

	
	while (n / 16 > 0)
	{
		count++;
		n = n / 16;
	}
	
	if (n % 16 != 0)
	{
		return count + 1;
	}
	else
	{
		return count;
	}
}
