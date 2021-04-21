/*----------------------------------------------------------------------------
    Code for Lab 6

    In this project the PIT and the TPM are used to create a tone. We will be storing 12 different tone within an array 
 *---------------------------------------------------------------------------*/

#include "cmsis_os2.h"
#include <MKL25Z4.H>
#include "../include/gpio.h"
#include "../include/pit.h"
#include "../include/tpmPwm.h"

osEventFlagsId_t evtFlags ; // event flags
   // Flag PRESS_EVT is set when the button is pressed

/*--------------------------------------------------------------
 *     Tone task - Change the tone by updating values in the PIT peripheral
 *--------------------------------------------------------------*/

osThreadId_t t_tone;        /*  task id of task to flash led */

void toneTask (void *arg) {
	
	const int maxNotes = 12; //array size
	int counter=0; //first element in the array
		
	const uint32_t midiNotes[] = {20040,18915,17853,16851,15905,15013,14170,13375,12624,11916,11247,10616};
	

    while (1) {
        osEventFlagsWait (evtFlags, MASK(PRESS_EVT), osFlagsWaitAny, osWaitForever); //await for button signal to change tone
			
				stopTimer(0);
				setTimer(0, midiNotes[counter]) ; //update the PIT peripheral with new computed values based off equation
				startTimer(0) ;
				
				counter++; //move on the next tone frequency
				if(counter>(maxNotes-1))
				counter = 0;
			
        }
    }

/*------------------------------------------------------------
 *     Button task - poll button and send signal when pressed
 *------------------------------------------------------------*/

osThreadId_t t_button;      /* task id of task to read button */

void buttonTask (void *arg) {
    int bState = UP ;
    int bCounter = 0 ;
    
    while (1) {
        osDelay(10) ;
        if (bCounter) bCounter-- ;
        switch (bState) {
            case UP:
                if (isPressed()) {
                    osEventFlagsSet(evtFlags, MASK(PRESS_EVT));
                    bState = DOWN ;
                }
                break ;
            case DOWN:
                if (!isPressed()) {
                    bCounter = BOUNCEP ;
                    bState = BOUNCE ;
                }
                break ;
            case BOUNCE:
                if (isPressed()) {
                  bCounter = BOUNCEP ;
                    bState = DOWN ;
                } else {
                    if (!bCounter) {
                        bState = UP ;
                    } 
                }
                break ;
        }
    }
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
    configureGPIOinput() ;       // Initialise button
    configureGPIOoutput() ;      // Initialise output    
    configurePIT(0) ;            // Configure PIT channel 0
    setTimer(0, 20040) ; // Frequency for MIDI 60 - middle C
    configureTPM0forPWM() ;
    setPWMDuty(64) ;     // 50% volume
												// Max is 128; off is 0 
    SystemCoreClockUpdate() ;

		
	
    // Initialize CMSIS-RTOS
    osKernelInitialize();
    
    // Create event flags
    evtFlags = osEventFlagsNew(NULL);

    // Create threads
    t_tone = osThreadNew(toneTask, NULL, NULL) ;
    t_button = osThreadNew(buttonTask, NULL, NULL) ;    
    
    
    osKernelStart();    // Start thread execution - DOES NOT RETURN
    for (;;) {}         // Only executed when an error occurs
}
