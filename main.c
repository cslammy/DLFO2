/* dirty digial LFO 8-9-21
this version assumes you are using the SMD pots board
otherwise you want to use the 5-21-21 code
AUDIO DIWHY
*/

//#define BAUD 9600
//#define BAUD_TOL 2

#include <avr/io.h>
#include "spi3.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h> // provides rand()
#include <util/delay.h>
#include "ADClib.h"
//#include "stdio_setup.h"
#include <stdio.h>
#include <math.h>
#include <avr/interrupt.h>
#include "MCP4921.h"



uint16_t WFMFREQ = 0; // increases max LFO speed at the expense of DAC resolution
uint16_t CV = 0;

volatile uint16_t c = 0; // counter in ISR--for DAC
volatile uint8_t waveselect = 0; //  **SET DEFAULT WAVE HERE** counter in ISR -- debounce wave switch
volatile uint8_t sh = 0;
//void SPI_TransferTx16(uint8_t a, uint8_t b);  // debug with stdio_setup.h printf to ser

 // saw does not begin at 0 so we need global variable.
uint16_t sawseed = 4096;
uint16_t *sawcount = &sawseed; // saw does not begin at 0 so we need global variable
 
void ramp(uint16_t freq);
void tri(uint16_t freq);
void squarewave(uint16_t freq);
void saw(uint16_t freq);
void randy(uint16_t freq);
void randy2(uint16_t freq);
void shold(uint16_t freq);

volatile uint16_t count = 0; // global counter for creating waveforms 
int16_t temp = 0;
uint8_t speed = 0;

 

//ISR routines have to go BEFORE main{}

ISR (TIMER0_COMPA_vect)  // timer0 overflow interrupt
{
	c++;
	
}

ISR (TIMER2_COMPA_vect)  // timer2 overflow interrupt
{
	if ((PIND & 0b00000010) == 0)
	{
	_delay_ms(500);
	waveselect++;
	
	}
	if (waveselect >= 7)  // one more than max waveselect value
	{
	waveselect = 0;	
	
	}
	
	
}

ISR (INT1_vect)
{
    //PORTB ^= (1 << 0);
	count = 4096; 
    *sawcount = 4096;
	sh = 10;
	
	_delay_ms(2);// "Max Riley Reset" new feature, incoming interrupt resets C (LFO wavefrm) to zero

}



int main(void)
{
	/*  pin D0 for course speed change we will change OCR0A value */
	/* pin D1 for waveform select NOTE!! for AVR GPIO 1 is output, 0 is input */
	
	DDRB = 0b00000001;  // TEST have B0 flash with interrupt
	
	DDRD = 0b11100000;      //make an input D7, D6, D5 outputs all others are inputs (old LED setup)
	
    PORTD = 0b00000011;    //enable pull-up for D0 and D1; this seems to trump I/O designation for DDRD register?
	/*  higher freq, less rez DEBUG, trying things out.....
	WFMFREQ adds to c count, more speed but less resolution for DAC.
	//WFMFREQ = 300; // for triangle you only get 4 steps at 300 WFMFREQ!!!
    //WFMFREQ = 20;
    //WFMFREQ = 10; */
	WFMFREQ = 12;  
	
	//WFMFREQ = 0;	
	//set up stdio_setup
	//UartInit();
	ADC_init();
    //initialize SPI
    
	init_spi_master();
    spi_mode(0);
	DESELECT();
	SELECT();
	
    //********RESET ISR ************    

	EICRA |= (1<<ISC01); // interrupt on falling edge (trig)
	//EICRA |= ~(1 << ISC10); // interrupt on low (trig)
	//EICRA |= 1 << ISC10; // interrupt on rising edge
    EIMSK |= 1<<INT0;  // enable external interrupt 1 (D2 pin)
	
 

	//********TIMER--FREQ************
	
	// Set the Timer 0 Mode to CTC
	TCCR0A |= (1 << WGM01);

	// Set the value that you want to count to
	OCR0A = 0xFF;

	TIMSK0 |= (1 << OCIE0A);    //Interrupt fires when counter matches OPR0A value above.

	//sei();         //enable interrupts  
	// in atmel s7 sei() is flagged as red squiggle due to 
	//"intellisence" beautifying,but will still compile. Doh!


    //TCCR0B  0 0 0 turns off clock, don't use!
	// CS2  CS1   CS0
	// 0     0     1  no prescale
	// 0     1     0  divide by 8
	// 0     1     1  divide by 64
	
    TCCR0B &= ~(1 << CS02); //CS2 to 0
	TCCR0B &= ~(1 << CS01);
	TCCR0B |= (1 << CS00);
	// set preschooler to no divide.
	
    //********END TIMER--FREQ************
	
	
	//********TIMER--WAVEFORM************
	//Use this to debounce momentary switch we will use for waveform select
	
	// Set the Timer 2 Mode to CTC
	TCCR2A |= (1 << WGM01);

	// Set the value that you want to count to
	OCR2A = 0xFF;

	TIMSK2 |= (1 << OCIE2A);    //Interrupt fires when counter matches OPR2A value above.

//	sei();         //enable interrupts  
	// in atmel s7 sei() is flagged as red squiggle due to 
	//"intellisence" beautifying,but will still compile. Doh!


    //TCCR2B  0 0 0 turns off clock, don't use!
	// CS2  CS1   CS0
	// 0     0     1  no prescale
	// 0     1     0  divide by 8
	// 1     0     0  divide by 64
	
    TCCR2B &= ~(1 << CS02); //CS2 to 0
	TCCR2B &= ~(1 << CS01);
	TCCR2B |=  (1 << CS00);
	// set preschooler to 64
		
   // ********END TIMER--WAVEFORM************	

   sei();         //enable interrupts 

//LED POWER UP FUN 


// logic for LEDs new for 8-9-21 board
      PORTD &= ~(1 << 5);     // turns LED 1 (BOTTOM) ON
      PORTD &= ~(1 << 6);    // turns MIDDLE LED2 ON
     PORTD  &= ~(1 << 7);    // TOP LED3 ON  
// _delay_ms(1000); //turn on all LEDs for 1 sec
 
    PORTD |= (1 << 5);     // turn off LEDs and wait
 //   _delay_ms(500);
	PORTD |= (1 << 6);
//	_delay_ms(500);
	PORTD |= (1 << 7);
//	_delay_ms(500);	
	
	
while(1)
		{
	   
	   speed = PIND;
	   if ((speed & 0b00000001) == 0x00)
		  {
		  OCR0A = 0xFF;
		  }
	   else
		  {
		  OCR0A = 0x01;
		  } 
	    
		CV = analogRead10bit();	    
			//ramp(CV);
			//tri(CV);
		
		if (waveselect == 0)
				{
				tri(CV);
				}
		
		if (waveselect == 1)
        		{
				ramp(CV);
        		}
		if (waveselect == 2)	 	
				{
				squarewave(CV);	
				}
        if (waveselect == 3)
        		{
	        	saw(CV);
		
        		}
        if (waveselect == 4)
                {
	            randy(CV); // random CV generator
                }

        if (waveselect == 5)
                {
	            randy2(CV); // random CV generator #2
                }
    
	    if (waveselect == 6)
	           {
		           shold(CV); // sample-hold
	           }

 
		}  // end while(1)
} // end main


void ramp(uint16_t freq)
{
    PORTD |= (1 << 5);     // BOTTOM LED1 off
	PORTD &= ~(1 << 6);    // MIDDLE LED2 on
	PORTD |= (1 << 7);    // TOP LED3 off
	uint16_t rate = 0;

	rate = 1024-freq;



		if (c >= rate)
		  {

  	

			 WFMFREQ = 12*(freq/100);
			   //next 2 lines for debug   
              //printf("ramp rate: %d",rate);
             // printf("ramp ccccc: %d",c); 	      
       
			 count = count + WFMFREQ;
	         uint8_t CMSB = count >> 8;
			 uint8_t CLSB = count & 0xFF;
		     write4921(CMSB,CLSB);
			 count++;
			 
			 
			 //next 2 lines for debug
			// printf("ramp count: %d \n\r",count);
			// printf("ramp bytes %x %x \n\r",CMSB,CLSB);
		     c = 0; 
  

  
  
             if (count > 4095)
             {
	             count = 0;
             }	
			 
 	  
		  }
		
	  
}

void squarewave(uint16_t freq)
{
		 PORTD |= (1 << 5);     // BOTTOM LED1 off
		 PORTD |= (1 << 6);    // MIDDLE LED2 off
		 PORTD &= ~(1 << 7);    // TOP LED3 on
	uint16_t rate = 0;

	rate = 1024-freq;

	WFMFREQ = freq/3;
		   

	if (c >= rate)
	{
		
		
		count = count + WFMFREQ;
		if (count > 2047)
		{

		write4921(0,0);
		
		}
		
		else
		{
		write4921(0x0F,0xFF);	
		}
		count++;
		//next 2 lines for debug
		// printf("sqwave count: %d \n\r",count);
		// printf("sqwave bytes %x %x \n\r",CMSB,CLSB);
		c = 0;
		

		
		if (count > 4096-WFMFREQ)
		{
			count = 0;
			
		}
		

	}
	
	
	if (c >= 25000) 
	{
		c = 0;
    }

	
}

 void tri(uint16_t freq)
 {
	 
	    // set pin 3 of Port B as output
	 PORTD &= ~(1 << 5);     // BOTTOM LED1 on
	 PORTD |= (1 << 6);    // MIDDLE LED2 off
	 PORTD |= (1 << 7);    // TOP LED3 off
	 uint16_t rate = 0;
     
	 rate = 1024-freq;
     WFMFREQ = 8*(freq/100);



	 if (c >= rate)
	 {
	 
		 //next 2 lines for debug
		 //printf("input rate: %d",rate);
		 // printf("tri ccccc: %d",c);
		 
 
		 
		 if (count < 4096 + WFMFREQ)
		    {
		    count = count + WFMFREQ;	
			uint8_t CMSB = count >> 8;
			uint8_t CLSB = count & 0xFF;
			write4921(CMSB,CLSB);
			count++;
			  	
			}
		

		 if (count >= 4096 + WFMFREQ)
			{		     
            
			 count = count + WFMFREQ;
			  
			 uint16_t triout = 4096 - (count - 4096);
			 uint8_t CMSB = triout >> 8;
			 uint8_t CLSB = triout & 0xFF;
			 write4921(CMSB,CLSB);
             count++;
    			
		    			}
         if (count > 8192 + WFMFREQ)
		 {
			
	         count=0;
	         
		 }
		 
 
		 
		 c = 0;

	 }
	 
 }
 
 void saw(uint16_t freq)
 {
	 PORTD &= ~(1 << 5);     // BOTTOM LED1 on
	 PORTD &= ~(1 << 6);    // MIDDLE LED2 on
	 PORTD |= (1 << 7);    // TOP LED3 off
     uint16_t rate = 0;
 			   
 	   speed = PIND;
 	   if ((speed & 0b00000001) == 0x00)
 	   {
	 	   _delay_ms(1);
 	   }
 	   else
 	   {
	 	    
 	   }
 

	//if (c >= rate)
    		
    		
    		
    		//next 2 lines for debug
    		// printf("ramp count: %d \n\r",count);
    		// printf("ramp bytes %x %x \n\r",CMSB,CLSB);
    		//c = 0;
    		
 

	if ((c >= 0) && (c < 65000))
	{
		 
	  
		//next 2 lines for debug
		//printf("ramp rate: %d",rate);
		// printf("ramp ccccc: %d",c);
	     
                     rate = freq + 1;
                
	                *sawcount = *sawcount - rate ;
	                
	                //x = 0;
                    uint8_t CMSB = *sawcount >> 8;
                    uint8_t CLSB = *sawcount & 0xFF;
                    write4921(CMSB,CLSB);
          
	
				
				if (*sawcount <= rate)
				{
					*sawcount = 4096;
					c = 0;
					//x = 0;
					
				}	
		
 
	
	}
	else
	{
		c = 0;
	}
	 
	 
 }
 void randy(uint16_t freq)
 {
	 PORTD &=  ~(1 << 5);     // BOTTOM LED1 on
	 PORTD |= (1 << 6);    // MIDDLE LED2 off
	 PORTD &= ~(1 << 7);    //  TOP LED3 on
	 
	 	   	   speed = PIND;
	 	   	   if ((speed & 0b00000001) == 0x00)
	 	   	   {
		 	   	   OCR0A = 0x01;
	 	   	   }
	 	   	   else
	 	   	   {
		 	   	   OCR0A = 0xFF;
	 	   	   }
	 
     
	 uint16_t rand1 = 0;
     uint16_t count = 0;
	 uint16_t delayfactor = 0;
	 
	 
     rand1= rand()  % 4096; // generate random number
	
	 c = 0;
	 
		    delayfactor = (1024 - freq)+50;
			
			 for (uint8_t del = 0; del < delayfactor/10; del++)
		     {
			_delay_ms(1);
			}
		 
		 count = rand1;
		 
		 if (c >= (4096 - 4*freq))
			 {
			 //x = 0;
			 uint8_t CMSB = count >> 8;
			 uint8_t CLSB = count & 0xFF;
			 write4921(CMSB,CLSB);
			 
			 if (sh == 10) //riley reset
			 {
				 		   
				 		    
				 		    for (uint32_t del = 0; del < (20 * (rand1 + 500)); del++)
				 		    {
					 		 			 uint8_t CMSB = count >> 8;
					 		 			 uint8_t CLSB = count & 0xFF;
					 		 			 write4921(CMSB,CLSB);  
				 		    }
				sh = 0;			 
			 }
			 
			 
			 c = 0;

			 }
		 
}
		 
		 
 
		 


	 
 
 
  void randy2(uint16_t freq)
   {
	   	   speed = PIND;
	   	   if ((speed & 0b00000001) == 0x00)
	   	   {
		   	   OCR0A = 0x01;
	   	   }
	   	   else
	   	   {
		   	   OCR0A = 0xFF;
	   	   }
	   
	   PORTD |= (1 << 5);     // LED1 on
	   PORTD &= ~(1 << 6);    // LED2 on
	   PORTD &= ~(1 << 7);    // LED3 off
	   
	   uint16_t rand1 = 0;
	   uint16_t count = 0;
	   //uint16_t freqflip = 0;
	   
	   rand1= rand()  % 4096; // generate random number
	   
	   

	   if ((c >= 0) && (c < 4096))
	   {
		   
		   
		   //next 2 lines for debug
		   //printf("ramp rate: %d",rate);
		   // printf("ramp ccccc: %d",c);
		   
		   
		   
		   count = rand1;
		   
		   if (c >= (1024 - freq))
		   {
			   //x = 0;
			   uint8_t CMSB = count >> 8;
			   uint8_t CLSB = count & 0xFF;
			   write4921(CMSB,CLSB);
			   
			   //freqflip = 1126 - freq;
			   
			   for (uint32_t waste = 0; waste < (4000/freq * (rand1*5)); waste++)
			   	{
			    //DO SQUAT
			 
				}
				
				
			    c = 0;
		   }
	   }

   }
	   
	   
	   


	   

   
   void shold(uint16_t freq)
   {

	  
	  //SH
	  	 PORTD &= ~(1 << 5);     // BOTTOM LED1 on
	  	 PORTD &= ~(1 << 6);    // MIDDLE LED2 on
	  	 PORTD &= ~(1 << 7);    //  TOP LED3 on
	  
	  if (sh == 10)
	   {
	   
	   freq = freq*4;
	   uint8_t CMSB = freq >> 8;
	   uint8_t CLSB = freq & 0xFF;
	   write4921(CMSB,CLSB);
	   
	   sh = 0;
	   }
	   
   }