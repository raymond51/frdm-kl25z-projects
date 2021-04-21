#include <stdbool.h>
#include "../include/gpio_defs.h"
// Definitions for GPIO
#ifndef GPIO_H
#define GPIO_H

// MASKing
#define MASK(x) (1UL << (x))

// Function prototypes
void configureGPIOinput(void) ;       // Initialise button
void configureGPIOoutput(void) ;      // Initialise output	
void callMotorUpdate(void);

#endif
