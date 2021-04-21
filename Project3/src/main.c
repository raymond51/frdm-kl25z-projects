#include <MKL25Z4.H>
#include <stdbool.h>
#include "SysTick.h"
#include "gpio.h"

/* ------------------------------------------
             ECS642 Lab3
             ===========
   Timing of cycle and percentage exceution time
   Reaction timer. 
   -------------------------------------------- */
 


int countGreen ; 
int countRed ;


int currentState; //defines the current state in the state machine
int setDelay; //Holds the randomly generated delay, this variable does not decrement
uint32_t randomDelay;				
int countDelay; //Count the duration of button delay
uint32_t countDelayButtom1ms; //accurate 1ms variable which holds button delay


//define all the functions

void configureInput(void);
void configureOutput();
void stateMachine();
void generateRandomValue();
uint32_t rand900(uint32_t r);
uint32_t nextRand(void);
void initGreenLED();
void initRedLED();


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
  configureInput() ;  // configure the GPIO input for the button
  configureOutput() ;  // configure the GPIO outputs for the LED 
  initRedLED() ; // initialise flash red task
	initGreenLED();// init green task
  Init_SysTick(1000) ; // initialse SysTick every 1ms
  waitSysTickCounter(10) ; // cycle every 10ms
	
	
	
	//define the initial variable state to be 0
	countGreen = 0; 
  countRed = 0;
	currentState = 0;
	setDelay = 0;
	countDelay=0;
	countDelayButtom1ms =0; 
		
  while (1) {
	
		stateMachine();
	
    waitSysTickCounter(10) ; // wait to end of cycle
  }
}


/*----------------------------------------------------------------------------
  Turn LEDs on or off 
    onOff can be ON or OFF
*----------------------------------------------------------------------------*/
void setRedLED(int onOff) {
  if (onOff == ON) {
    PTB->PCOR = MASK(RED_LED_POS) ;               
  } 
  if (onOff == OFF) {
    PTB->PSOR =  MASK(RED_LED_POS) ;
  }
  // no change otherwise
}

void setGreenLED(int onOff) {
  if (onOff == ON) {
    PTB->PCOR = MASK(GREEN_LED_POS) ;              
  } 
  if (onOff == OFF) {
    PTB->PSOR = MASK(GREEN_LED_POS) ;
  }
  // no change otherwise
}

void setBlueLED(int onOff) {
  if (onOff == ON) {
      PTD->PCOR = MASK(BLUE_LED_POS) ;              
  } 
  if (onOff == OFF) {
      PTD->PSOR = MASK(BLUE_LED_POS) ;
  }
  // no change otherwise
}

/*----------------------------------------------------------------------------
 * Interrupt Handler GPIO A
 *    - Clear the pending request
 *    - Test the bit to see if it generated the interrupt 
  ---------------------------------------------------------------------------- */

// Button signal
//   Set in the interupt handler
//   Cleared by the task in the main cycle
volatile uint32_t buttonSignal=0;

void PORTA_IRQHandler(void) {  
  NVIC_ClearPendingIRQ(PORTA_IRQn);
  if ((PORTA->ISFR & MASK(BUTTON_POS))) {
    // Add code to respond to interupt here
    buttonSignal = 1 ;

  }
  // Clear status flags 
  //PORTA->ISFR = MASK(BUTTON_POS) ; 
	// clear status flags 
	PORTA->ISFR = 0xffffffff;
}


void initRedLED() {
  countRed = RED_PERIOD ;
  setRedLED(ON) ;
}

void initGreenLED() {
  countGreen = GREEN_PERIOD ;
  setGreenLED(OFF) ;
}


/*----------------------------------------------------------------------------
 * nextRand: get next random number 
 *   Based on https://en.wikipedia.org/wiki/Linear_congruential_generator
 * --------------------------------------------------------------------------- */
uint32_t seed = 0x12345678 ;

// Returns a 32 bit number which is too long for us
uint32_t nextRand(void) {
  seed = (1103515245 * seed + 12345) ; 
  return seed ; 
}


// Generate random in range 0 to 900
//    - take top 10 bits - max is 1023
//    - reject if > 900 (about 10% probability)
uint32_t rand900(uint32_t r) {
  uint32_t r900 = (r & 0xFFC00000) >> 22 ; // top 10 bits
  while (r900 > 900) r900 = (nextRand() & 0xFFC00000) >> 22 ;
  return r900 ;
}

void generateRandomValue(){
	 // get random in range 0 to 900
        randomDelay = nextRand() ; // 32 bit random
        randomDelay = rand900(randomDelay) ; // reduce range to 0 to 900

        countDelay = randomDelay + 100 ;   // delay range 1 - 10 sec
				setDelay = countDelay; //store in another variable where it holds count without decrementing
}

void stateMachine(){

	//random function generator is not properly functioning
	
switch(currentState){

	case readyState:
		if(buttonSignal){
			buttonSignal = 0; //acknowledge button press
			
			setRedLED(OFF); //let user know that the system acknowledge button press and will begin the reaction time example
			currentState = RandomWaitState;
		
		}
			generateRandomValue(); //Code repeatedly as if when called once will generate the same delay
		  break;
	case RandomWaitState:
		
	//buttonSignal being stuck at 1 due to button bouncing
	
		if (countDelay > 0)
				countDelay -- ;
	
	
			if((setDelay - countDelay)> 20){ // register delay to account for the button debounce
			if(buttonSignal){
				buttonSignal=0;
			currentState = ErrorOnState;
			}
			}else{
				buttonSignal=0; //clear the signal as button bouncing may occur below 200ms
		}
	
			
		if(countDelay==0){
			buttonSignal = 0; // this is required otherwise the green led goes to flashing state due to button bounce
			setGreenLED(ON);
			ButtonCounter1ms = setDelay;
			currentState = TimingState;
		}

break;		
		
	case TimingState:
		
		if(buttonSignal){
		buttonSignal = 0;
		countDelayButtom1ms = setDelay - ButtonCounter1ms;
		currentState = SuccessOffState;
		}else{
		countDelay += 1;
		}
		
	break;
		
	case ErrorOnState:
		  if (countRed > 0) countRed -- ;
		if (countRed == 0) {   
        setRedLED(OFF) ;
        currentState = ErrorOffState ;
        countRed = RED_PERIOD ;
      } 
	break;
	
	case ErrorOffState:
		  if (countRed > 0) countRed -- ;
      if (countRed == 0) {
        setRedLED(ON) ;
        currentState = ErrorOnState ;
        countRed = RED_PERIOD ;
      }
	
	break;
	
	case SuccessOffState:
	  if (countGreen > 0) countGreen -- ;
	 if (countGreen == 0) {
        setGreenLED(ON) ;
        currentState = SuccessOnState ;
        countGreen = GREEN_PERIOD ;
      }
	break;
	
	case SuccessOnState:
		  if (countGreen > 0) countGreen -- ;
	      if (countGreen == 0) {   
        setGreenLED(OFF) ;
        currentState = SuccessOffState ;
        countGreen = GREEN_PERIOD ;
      } 
	
	break;

}

}

/*----------------------------------------------------------------------------
  Configuration 
     The configuration of the GPIO port is explained in week 2
     Enabling the clocks will be covered in week 3.
     Configuring the PORTx peripheral, which controls the use of each pin, will
       be covered in week 3
*----------------------------------------------------------------------------*/
void configureOutput() {
  // Configuration steps
  //   1. Enable clock to GPIO ports
  //   2. Enable GPIO ports
  //   3. Set GPIO direction to output
  //   4. Ensure LEDs are off

  // Enable clock to ports B and D
  SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;
    
  // Make 3 pins GPIO
  PORTB->PCR[RED_LED_POS] &= ~PORT_PCR_MUX_MASK;          
  PORTB->PCR[RED_LED_POS] |= PORT_PCR_MUX(1);          
  PORTB->PCR[GREEN_LED_POS] &= ~PORT_PCR_MUX_MASK;          
  PORTB->PCR[GREEN_LED_POS] |= PORT_PCR_MUX(1);          
  PORTD->PCR[BLUE_LED_POS] &= ~PORT_PCR_MUX_MASK;          
  PORTD->PCR[BLUE_LED_POS] |= PORT_PCR_MUX(1);          

  // These lines make another pin of PTB an output 
  // Use this for the oscilloscope timing measurement
  PORTB->PCR[EXTERNAL_POS] &= ~PORT_PCR_MUX_MASK;  
  PORTB->PCR[EXTERNAL_POS] |= PORT_PCR_MUX(1);   

  // Set ports to outputs
  PTB->PDDR |= MASK(RED_LED_POS) | MASK(GREEN_LED_POS) | MASK(EXTERNAL_POS) ;
  PTD->PDDR |= MASK(BLUE_LED_POS);

  // Turn off LEDs and external output
  PTB->PSOR = MASK(RED_LED_POS) | MASK(GREEN_LED_POS) ;
  PTD->PSOR = MASK(BLUE_LED_POS) ;
  PTB->PCOR = MASK(EXTERNAL_POS) ;
}

/*----------------------------------------------------------------------------
  GPIO Input Configuration

  Initialse a GPIO port A pin 4 as an input (GPIO data direction register)
  Bit number given by BUTTON_POS
  Configure the pin with an interrupt on the falling edge and a pull up resistor is enabled.
 *----------------------------------------------------------------------------*/
// 
void configureInput(void) {
  SIM->SCGC5 |=  SIM_SCGC5_PORTA_MASK; /* enable clock for port A */

  /* Select GPIO and enable pull-up resistors and no interrupts */
  PORTA->PCR[BUTTON_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | 
           PORT_PCR_IRQC(0x0A) | PORT_PCR_PFE_MASK ;
      
  /* Set port D switch bit to be an input */
  PTA->PDDR &= ~MASK(BUTTON_POS);
  
  /* Enable Interrupts */
  NVIC_SetPriority(PORTA_IRQn, 128); // 0, 64, 128 or 192
  NVIC_ClearPendingIRQ(PORTA_IRQn);  // clear any pending interrupts
  NVIC_EnableIRQ(PORTA_IRQn);
}







