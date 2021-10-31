#ifndef ADCLIB_H_
#include <stdint.h>
//LIBRARY to do 10 or 8 bit reads from Atmel MPU ADC

//initialize ADC (set sample rate etc).
//no return.

void ADC_init();

//return 8 bit value.  This is
//contents of 8 LSBs.

uint8_t analogRead8bit();

//return an int of 10 LSBs from ADC.
//don't forget to set one of the analog portCs as input.
//otherwise this won't work.
//hard coded to use MUX channel 0.
uint16_t analogRead10bit();


#endif