/*
 * MCP4921.c
 *
 * Created: 3/22/2021 11:07:45 AM
PUT THESE 2 lines onto main.c {} file but before while(1) loop.


init_spi_master();
spi_mode(0);




 *  Author: clamm
*/

#include <stdio.h>
#include "MCP4921.h"
#include "spi3.h"

uint8_t MSBwcontrol = 0;

void write4921(uint8_t MSB, uint8_t LSB)
    {
	

		
		

		MSBwcontrol = 0b01110000 + MSB; // control bits: write to DAC; buffer out; normal gain; no shutdown;  bit 15 is always 0
		
		
	    // for 4921, you send 2 bytes.  bit 15 is always zero,
	    // bits 14,13,12 are control, 11,10,9,8 forms MSB of data sent.
	    // bits 7-0 are LSB.
	     

	    //send the data.  4921 can eat 16 bit xfers.
	    SELECT();

        SPI_SendByte(MSBwcontrol);

		SPI_SendByte(LSB);
	    
		DESELECT();
	
	    
    }
