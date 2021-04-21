#include <MKL25Z4.H>
#include <stdbool.h>
#include "../include/gpio.h"


/*----------------------------------------------------------------------------
  GPIO Input Configuration

  Initialse a Port D pin as an input, with no interrupt
  Bit number given by BUTTON_POS
 *----------------------------------------------------------------------------*/
void configureGPIOinput(void) {
    SIM->SCGC5 |=  SIM_SCGC5_PORTD_MASK; /* enable clock for port D */

    /* Select GPIO and enable pull-up resistors and no interrupts */
    PORTD->PCR[BUTTON_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0);
    
	
		PORTD->PCR[BUTTON_POS_STOP] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0);
    /* Set port D switch bit to inputs */
    PTD->PDDR &= ~MASK(BUTTON_POS);
	
		PTD->PDDR &= ~MASK(BUTTON_POS_STOP); //add button stop
}

/* ----------------------------------------
     Configure GPIO output 
       1. Enable clock to GPIO port
       2. Enable GPIO port
       3. Set GPIO direction to output
       4. Ensure output low
 * ---------------------------------------- */
void configureGPIOoutput(void) {
    // Enable clock to port A
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    
		SIM->SCGC5 |=  SIM_SCGC5_PORTD_MASK; /* enable clock for port D */
	      
	 /* Select GPIO and enable pull-up resistors and no interrupts */
    PORTD->PCR[BUTTON_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0);
	
	  /* Set port D switch bit to inputs */
    PTD->PDDR &= ~MASK(BUTTON_POS);

}