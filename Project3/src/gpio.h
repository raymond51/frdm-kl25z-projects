#ifndef GPIO_H
#define GPIO_H

// Create a bit mask (32 bits) with only bit x set
#define MASK(x) (1UL << (x))

// Freedom KL25Z LEDs pin numbers
#define RED_LED_POS (18)		// on port B
#define GREEN_LED_POS (19)	// on port B
#define BLUE_LED_POS (1)		// on port D

#define EXTERNAL_POS (0)	// on port B

// Switches is on port A, pin 12
#define BUTTON_POS (12)

// Symbols for constants
#define OFF 0 
#define ON 1 
#define RED_PERIOD 100 // time in 10ms units
#define GREEN_PERIOD 100 // time in 10ms units

// States - task flashRed
#define REDON 0 
#define REDOFF 1

// States - task lightBlue
#define BLUEON 0
#define BLUEOFF 1

// States - task flashRed
#define GREENON 0 
#define GREENOFF 1

//state machine

#define readyState 0
#define RandomWaitState 1
#define TimingState 2 
#define ErrorOnState 3
#define ErrorOffState 4
#define SuccessOffState 5
#define SuccessOnState 6

#endif
