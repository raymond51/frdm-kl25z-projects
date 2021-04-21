#include <stdbool.h>
#include <stdint.h>

#ifndef GPIO_DEFS_H
#define GPIO_DEFS_H

#define MASK(x) (1UL << (x))

// Freedom KL25Z LEDs
#define RED_LED_POS (18)		// on port B
#define GREEN_LED_POS (19)	// on port B
#define BLUE_LED_POS (1)		// on port D

// Switches is on port D, pin 6
#define BUTTON_POS (6)
#define BUTTONUP 1 
#define BUTTONDOWN 2 
#define BUTTONBOUNCE 3
#define BOUNCEDELAY 5      // cycles to delay for 

// LED states
#define LED_ON (1)
#define LED_OFF (0)

// Symbols for constants
#define OFF 0 
#define ON 1 

// timings for BLue LED
#define BLUELED_TOTALTIME 400.0
#define BLUELED_MIN_ONTIME 20.0
#define BLUELED_MAX_ONTIME 360.0

// define blue on period for error state
#define BLUE_PERIOD 50

// Function prototypes
void init_LED(void) ;
void init_ButtonGPIO(void) ;
bool isPressed(void) ;
void setBlueLED(int onOff);
void redLEDOnOff (int onOff);
void greenLEDOnOff (int onOff);

#endif
