// Header file for GPIO in Lab 5
//   Definitions for pin usage
//   Function prototypes

#ifndef GPIO_DEFS_H
#define GPIO_DEFS_H

#include <stdbool.h>


#define MASK(x) (1UL << (x))

// Freedom KL25Z LEDs
#define RED_LED_POS (18)    // on port B
#define GREEN_LED_POS (19)	// on port B
#define BLUE_LED_POS (1)	// on port D

// Button is on port D, pin 6
#define BUTTON_POS (6)

#define BUTTONUP (0)
#define BUTTONDOWN (1)
#define BUTTONBOUNCE (2)

#define PRESS_EVT (0) // signal number
#define BOUNCE_COUNT (4)

// LED states
#define LED_ON (1)
#define LED_OFF (0)

// Function prototypes
void configureGPIOoutput(void) ;
void configureGPIOinput(void) ;
void redLEDOnOff (int onOff) ;
void greenLEDOnOff (int onOff) ;
bool isPressed(void) ;

#endif


