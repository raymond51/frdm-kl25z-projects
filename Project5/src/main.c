/*----------------------------------------------------------------------------
    Code for Lab 5

    In this project there are either lights on in a single instance, a button will be acknowledge to record the timing period for each led color. Once the timing is set, color led alight for a duration of time, when the button is pressed the timing is reset.
--------------------------------------------------------------------*/
 
#include "cmsis_os2.h"
#include <MKL25Z4.h>
#include <stdbool.h>
#include "gpio.h"

osEventFlagsId_t evtFlags ;          // event flags


/*--------------------------------------------------------------
 *   Thread t_RedandGreenLED
 *      Thread to handle both the green and red led, the state machine governs the flow of button press
 *--------------------------------------------------------------*/


osThreadId_t t_RedandGreenLEDStateMachine;      /* id of thread to toggle green led */

void RedandGreenLEDThread (void *arg) {
	//states
		#define readyState 0
		#define measure_Redperiod 1
		#define measure_Greenperiod 2
		#define GREENON 3 
		#define REDON 4
		
		uint32_t tick; //holds the tick counter
		uint32_t tickRedPeriod; //store the tick when button pressed again
		uint32_t tickGreenPeriod;
	
		uint32_t redPeriod; //hold the 
		uint32_t greenPeriod;
	
		uint32_t ledDelay; //variable constantly updated by the state machine to update the led delay timeout
	
		uint32_t flags; //flag to hold the value returned by the osEventFlagWait
		int flashEnabled = 0; //used as an flag to determine if the statemachine has measured green and red period
	
	
		int currentState = readyState;
		
		//initially no led color on
    greenLEDOnOff(LED_OFF);	
		redLEDOnOff(LED_OFF); 
			
			//loop
			while(1){
			
			if(flashEnabled){
       flags = osEventFlagsWait(evtFlags, MASK(PRESS_EVT), osFlagsWaitAny, ledDelay);
			}
			
			
	//flags needs to be checked when time out to determine what triggered
	//statement to read what values returned from osEventWait, if not timeout do something
			if(flags == MASK(PRESS_EVT) ){
			
					currentState = REDON ; //reset to red led when button pressed
			
			}
				
				
				
		//switch to alternate between red and green led during timeout 
        switch (currentState) {
					case readyState:
						osEventFlagsWait (evtFlags, MASK(PRESS_EVT), osFlagsWaitAny, osWaitForever); //await for button press
						tick = osKernelGetTickCount(); //grab current tick count for counting period of red led
						redLEDOnOff(LED_ON); 
						currentState = measure_Redperiod;
						
					break;
					case measure_Redperiod:
						osEventFlagsWait (evtFlags, MASK(PRESS_EVT), osFlagsWaitAny, osWaitForever); //await for button press
					
						tickRedPeriod = osKernelGetTickCount(); //get current tick period after 
						redPeriod = tickRedPeriod - tick; //find the difference between wait duration
						tick = osKernelGetTickCount(); //grab current tick count for counting period of green led
						redLEDOnOff(LED_OFF); //turn off the led first then turn on the green led
						greenLEDOnOff(LED_ON);	
						currentState = measure_Greenperiod;
					
					
					break;
					case measure_Greenperiod:
						osEventFlagsWait (evtFlags, MASK(PRESS_EVT), osFlagsWaitAny, osWaitForever); //await for button press
					
						tickGreenPeriod = osKernelGetTickCount(); //get current tick period after button press
						greenPeriod = tickGreenPeriod - tick; //find the difference between wait duration
						flashEnabled = 1; //raise flag that we finished setting period for both led and green period
						greenLEDOnOff(LED_OFF);
						redLEDOnOff(LED_ON);
						//set the led color on and other color off 
						currentState = REDON; //start with red iniitially on 
					
					break;
					
				case GREENON:
						redLEDOnOff(LED_OFF);
						greenLEDOnOff(LED_ON);
						ledDelay = greenPeriod; 
						currentState = REDON ;
						break ;
				case REDON:
						greenLEDOnOff(LED_OFF);
						redLEDOnOff(LED_ON);
						ledDelay = redPeriod; 
						currentState = GREENON ;
						break ;
						
        }
  }
}


/*------------------------------------------------------------
 *  Thread t_button
 *      Poll the button
 *      Signal if button pressed
 *------------------------------------------------------------*/
osThreadId_t t_button;        /* id of thread to poll button */

void buttonThread (void *arg) {
    int state ; // current state of the button
    int bCounter ;
    state = BUTTONUP ;
  
    while (1) {
        osDelay(10);  // 10 ticks delay - 10ms
        switch (state) {
            case BUTTONUP:
                if (isPressed()) {
                    state = BUTTONDOWN ;
                    osEventFlagsSet(evtFlags, MASK(PRESS_EVT));
                }
                break ;
            case BUTTONDOWN:
                if (!isPressed()) {
                    state = BUTTONBOUNCE ;
                    bCounter = BOUNCE_COUNT ;
                }
                break ;
            case BUTTONBOUNCE:
                if (bCounter > 0) bCounter -- ;
                if (isPressed()) {
                    state = BUTTONDOWN ; }
                else if (bCounter == 0) {
                    state = BUTTONUP ;
                }
                break ;
        }
    }
}


/*----------------------------------------------------------------------------
 * Application main
 *   Initialise I/O
 *   Initialise kernel
 *   Create threads
 *   Start kernel
 *---------------------------------------------------------------------------*/
 
int main (void) { 
    // System Initialization
    SystemCoreClockUpdate();

    // Initialise peripherals
    configureGPIOoutput();
    configureGPIOinput();
 
    // Initialize CMSIS-RTOS
    osKernelInitialize();
    
    // Create event flags
    evtFlags = osEventFlagsNew(NULL);

    // Create threads
	
    t_RedandGreenLEDStateMachine = osThreadNew(RedandGreenLEDThread, NULL, NULL);
    t_button = osThreadNew(buttonThread, NULL, NULL); 
    
    osKernelStart();    // Start thread execution - DOES NOT RETURN
    for (;;) {}         // Only executed when an error occurs
}
