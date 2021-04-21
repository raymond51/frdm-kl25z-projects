/*  -------------------------------------------
 LAB4 Additional 

Description:
This code includes a state machine to control flow of execution

1. The DAC inputs are
       - Single: ADC0_SE8 (PTB0)
 
   
    The system uses code from 
       adc.c  
       SysTick.c  
       gpio.c - provide code to initialise and use GPIO button and LED
------------------------------------------- */
#include <MKL25Z4.H>
#include <stdbool.h>
#include <stdint.h>

#include "..\include\gpio_defs.h"
#include "..\include\adc_defs.h"
#include "..\include\SysTick.h"


//states in the state machine
#define measure_minimum_voltage 0
#define measure_maximum_voltage 1
#define error_on 2
#define error_off 3
#define blue_on 4
#define blue_off 5

int currentState; //defines the current state in the state machine
int countBlue ; //decrementer val for blue LED count

float vmax; //define value for maximum pot voltage
float vmin; //define value for minimum pot voltage

int time; //elapsed time 
float timeProportion; //variable holds the ratio of time duration out of 4 seconds
int continuousVoltageMeasurement; //variable to show success i.e vmax>vmin

// declare volatile to ensure changes seen in debugger
volatile float measured_voltage ;  // scaled value
float convertedVoltProportion; //holds the ratio of input vs vmin and vmax

/*  -----------------------------------------
@brief: Initialises the blue led variables and state
    -----------------------------------------   */
void initLightBlue() {
	time = 0;
	timeProportion=0;
  setBlueLED(OFF) ;
}

/*  -----------------------------------------
mapVoltage
@brief: Converts the measured voltage value from the ADC to equivalent proportion in time
    -----------------------------------------   */
float mapVoltage(float vin){
	if(vin <= vmin)
		return 0;
	else if(vin >= vmax)
		return 1;
	return ((vin-vmin)/(vmax-vmin));
}


/*  -----------------------------------------
measureVoltageSingle
@brief: makes a single adc reading and stores it in global variable
    -----------------------------------------   */

void measureVoltageSingle(){
	
	                // take a simple-ended voltage reading
          MeasureVoltage() ;    // updates sres variable
          // scale to an actual voltage, assuming VREF accurate
         measured_voltage = (VREF * sres) / ADCRANGE ;
	
				convertedVoltProportion = mapVoltage(measured_voltage);
}

/*----------------------------------------------------------------------------
  checkButton
@brief: This tasks polls the button and signals when it has been pressed
*----------------------------------------------------------------------------*/
int buttonState ; // current state of the button
int bounceCounter ; // counter for debounce
bool pressed ; // signal if button pressed

void init_ButtonState() {
    buttonState = BUTTONUP ;
    pressed = false ; 
}

void ButtonPress() {
    if (bounceCounter > 0) bounceCounter-- ;
    switch (buttonState) {
        case BUTTONUP:
            if (isPressed()) {
                buttonState = BUTTONDOWN ;
                pressed = true ; 
            }
          break ;
        case BUTTONDOWN:
            if (!isPressed()) {
                buttonState = BUTTONBOUNCE ;
                bounceCounter = BOUNCEDELAY ;
            }
            break ;
        case BUTTONBOUNCE:
            if (isPressed()) {
                buttonState = BUTTONDOWN ;
            }
            else if (bounceCounter == 0) {
                buttonState = BUTTONUP ;
            }
            break ;
    }                
}

/*----------------------------------------------------------------------------
 statemachine
@brief: This tasks handles the logic flow of the system
*----------------------------------------------------------------------------*/

void statemachine(){

	switch(currentState){

	case measure_minimum_voltage:
	if (pressed) {
                pressed = false ;     // acknowledge event
        
                // take a simple-ended voltage reading
                MeasureVoltage() ;    // updates sres variable
                // scale to an actual voltage, assuming VREF accurate
                vmin = (VREF * sres) / ADCRANGE ;
                
                // move onto next state
                currentState = measure_maximum_voltage ;
		            redLEDOnOff(LED_OFF) ;              
                greenLEDOnOff(LED_ON) ;

            }
		  break;
						
	case measure_maximum_voltage:
		if (pressed) {
                pressed = false ;     // acknowledge event
                // take a simple-ended voltage reading
                MeasureVoltage() ;    // updates sres variable
                // scale to an actual voltage, assuming VREF accurate
                vmax = (VREF * sres) / ADCRANGE ;
								greenLEDOnOff(LED_OFF) ;

			if(vmax>vmin){
				currentState = blue_on ;
				 setBlueLED(ON) ;
				continuousVoltageMeasurement=1; //enable continuous reading of input voltage as upper and lower bound voltage has been set
			}else{
				currentState = error_on ;
            }
		
		  break;		

	case error_on:
			  if (countBlue > 0) countBlue -- ;
				if (countBlue == 0) {   
        setBlueLED(OFF) ;
        currentState = error_off ;
        countBlue = BLUE_PERIOD ;
      } 
		  break;		
				
	case error_off:
	  if (countBlue > 0) countBlue -- ;
      if (countBlue == 0) {
        setBlueLED(ON) ;
        currentState = error_on ;
        countBlue = BLUE_PERIOD ;
      }
		  break;		
			
	case blue_on:
			time ++;
			timeProportion = (time-BLUELED_MIN_ONTIME)/BLUELED_MAX_ONTIME;
			if (timeProportion>=convertedVoltProportion){
        setBlueLED(OFF) ;
        currentState = blue_off ;
      }
		  break;	
			
	case blue_off:
			time ++;
	if (time >= BLUELED_TOTALTIME) {
        setBlueLED(ON) ;
        currentState = blue_on ;
				time = 0;
      }
		  break;	
		}	
}

}

/*----------------------------------------------------------------------------
 initLED
@brief: initialise colors of RGB LEDS when system reset
*----------------------------------------------------------------------------*/
void initLED(){
	//initial color states
		greenLEDOnOff(OFF);
		redLEDOnOff(ON);
		setBlueLED(OFF);
}

/*----------------------------------------------------------------------------
initGlobalVar
@brief: sets the default values of global variables
*----------------------------------------------------------------------------*/
void initGlobalVar(){
		continuousVoltageMeasurement =0;
		currentState = 0;
		vmin=0;
		vmax=0;
		countBlue =0;
		convertedVoltProportion=0; 
}



/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
volatile uint8_t calibrationFailed ; // zero expected
int main (void) {
	
    // Enable clock to ports B, D and E
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK ;

    init_LED() ; // initialise LED
    init_ButtonGPIO() ; // initialise GPIO input
		init_ButtonState();
    Init_ADC() ; // Initialise ADC
    calibrationFailed = ADC_Cal(ADC0) ; // calibrate the ADC 
    while (calibrationFailed) ; // block progress if calibration failed
    Init_ADC() ; // Reinitialise ADC
		initLED(); //initialise colors of RGB LEDS
		initLightBlue(); //initialise values for blue led
		initGlobalVar();
	
    Init_SysTick(1000) ; // initialse SysTick every 1ms
    waitSysTickCounter(10) ;

    while (1) {        
			
			ButtonPress();
			//if succcessful i.e vmax>vmin then statement below is true
			if(continuousVoltageMeasurement){
				measureVoltageSingle();
			}
			statemachine();
      // delay
      waitSysTickCounter(10) ;  // cycle every 10 ms
    }
}
