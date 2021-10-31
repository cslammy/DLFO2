#include <avr/io.h>
#include "ADClib.h"


void ADC_init()
{
	
	//USING THIS!
	//next line sets which C port is ADC in.
	//may not make sense to use all 8 pins as inputs...
	//but that's what I have here
	// NExt line configures all PORTC bits to an input
	DDRC = 0b00000000;


	//UNOS have AVCC pulled high (see schematic) but you may need to change that depending on app.
	//bit 7,6 bits set ADC reference voltage.  00 external  01 AVCC   10 reserved  11 internal 2.56V
	//bit 5 set to 1 = ADC left adjust.  0 right adjust
	//including gain shifts.
	// bits 4-0 determine which port is used for input
	// check AVR datasheet for how to set these to configure input ports.

	// Configure ADC to be right justified, use AVCC as reference, and select ADC1 as ADC input
	ADMUX = 0b01000001;

	// Enable the ADC and set the prescaler to max value (128)
	ADCSRA = 0b11000111;
}

uint8_t analogRead8bit()
{
	uint8_t x;
	uint16_t temp;
	temp = analogRead10bit();
	x = temp >> 2;
	
	
	return x;
}

uint16_t analogRead10bit()
{
	uint16_t y;
	uint16_t a;
	
	//get 8 bits and put into ADCH register.
	// Start an ADC conversion by setting ADSC bit (bit 6)
	ADCSRA = ADCSRA | (1 << ADSC);
	// Wait for ADSC bit to be cleared
	while(ADCSRA & (1 << ADSC));
	ADCH;
	
	ADCSRA = ADCSRA | (1 << ADSC);
	//get upper 2 bits to add to lower 8.
	
	// Wait until the ADSC bit has been cleared
	while(ADCSRA & (1 << ADSC));
	
	// make the output a 10 bit int.
	y = ADCH << 8;
	//y = ADCL;
	a = y + ADCL;

	return a;
}
