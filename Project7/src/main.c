
/* ------------------------------------------
LAB 7 Project

@brief: 
This project uses buttons to control the motion of the motor turning, we have pre-defined the number of cycle and speed, this is stored
in an array. We also have to compute the timing of the motor to get an accurate rotation frequency.


		Motor pin connections made
     -----------------------------
         IN1           pin 30       (phase A+)
         IN2           pin 29       (phase A-)
         IN3           pin 23       (phase B+)
         IN4           pin 22       (phase B-)

     -------------------------------------------- */

#include <MKL25Z4.H>
#include "../include/gpio_defs.h"
#include "../include/stepperMotor.h"
#include "../include/SysTick.h"
#include "../include/pit.h" //including pit for stepper motor control
#include "../include/gpio.h" //including gpio cofigurations
#include <stdbool.h>
#include <stdlib.h>

#define BUTTONOPEN (0)
#define BUTTONCLOSED (1)
#define BUTTONBOUNCE (2)


bool motorRunning = false ;

volatile int originalPosStep = 0;



/*----------------------------------------------------------------------------
  Motor Configuration

 *----------------------------------------------------------------------------*/
motorType mcb ;   // motor control block
MotorId m1 ;      // motor id
void configureMotor() {
    m1 = & mcb ;
    m1->port = PTE ;
    m1->bitAp = MOTOR_IN1 ;
    m1->bitAm = MOTOR_IN2 ;
    m1->bitBp = MOTOR_IN3 ;
    m1->bitBm = MOTOR_IN4 ;

    // Enable clock to port E
    SIM->SCGC5 |=  SIM_SCGC5_PORTE_MASK; /* enable clock for port E */
    
    // Initialise motor data and set to state 1
    initMotor(m1) ; // motor initially stopped, with step 1 powered
}

/* ----------------------------------------
     Update function triggered by pit interrupt
 * ---------------------------------------- */
void callMotorUpdate(void) {
    updateMotor(m1) ; //causes movement of steps
		motorRunning = isMoving(m1) ;
		originalPosStep++;
}

/*----------------------------------------------------------------------------
  isPressed: test the switch

  Operating the switch connects the input to ground. A non-zero value
  shows the switch is not pressed.
 *----------------------------------------------------------------------------*/
bool isPressed(void) {
    if (PTD->PDIR & MASK(BUTTON_POS)) {
            return false ;
    }
    return true ;
}

/*----------------------------------------------------------------------------
 Run button - currently being polled

 Detect changes in the switch state.
    isPressed and not closed --> new press; 
     ~isPressed and closed -> not closed
*----------------------------------------------------------------------------*/
int b_state = BUTTONOPEN ;
int pressed = 0 ;
int bounceCounter = 0 ;

void StartPollInput(void)
{
    if (bounceCounter > 0) bounceCounter -- ;
    
    switch (b_state) {
        case BUTTONOPEN :
            if (isPressed()) {
                pressed = 1 ;  // create a 'pressed' event
                b_state = BUTTONCLOSED ;
            }
            break ;
        case BUTTONCLOSED :
            if (!isPressed()) {
                b_state = BUTTONBOUNCE ;
                bounceCounter = 50 ;
            }
            break ;
        case BUTTONBOUNCE :
            if (isPressed()) {
                b_state = BUTTONCLOSED ;
            }
            if (bounceCounter == 0) {
                b_state = BUTTONOPEN ;
            }
            break ;
    }
}


/*----------------------------------------------------------------------------
Stop button
*----------------------------------------------------------------------------*/

bool isPressedD7(void) {
    if (PTD->PDIR & MASK(BUTTON_POS_STOP)) {
           return false ;
    }
    return true ;
}


int b_state_stop = BUTTONOPEN ;
int pressed_stop = 0 ;
int bounceCounter_stop = 0 ;

void StopPollInput(void)
{
    if (bounceCounter_stop > 0) bounceCounter_stop -- ;
    
    switch (b_state_stop) {
        case BUTTONOPEN :
            if (isPressedD7()) {
                pressed_stop = 1 ;  // create a 'pressed' event
                b_state_stop = BUTTONCLOSED ;
            }
            break ;
        case BUTTONCLOSED :
            if (!isPressedD7()) {
                b_state_stop = BUTTONBOUNCE ;
                bounceCounter_stop = 50 ;
            }
            break ;
        case BUTTONBOUNCE :
            if (isPressedD7()) {
                b_state_stop = BUTTONCLOSED ;
            }
            if (bounceCounter_stop == 0) {
                b_state_stop = BUTTONOPEN ;
            
            break ;
    }
}
}

int delay = 0;
/*
computePitCounter
@brief: Takes in total steps and time and returns the required value for the pit timer LDVAL to operate
*/
double computePitCounter(double totalStep,double time){
	double requiredFreq = 0;
	//time taken for 1 step
	requiredFreq = 1/(time/totalStep);
	//return the pit value LDV
	return (10485760/requiredFreq);
}

/* --------------------------------------
     Governs control of the stepper motor
   -------------------------------------- */
	//states
		#define readyState 0
		#define makeMove 1
		#define moveStop 2
		#define completeMove 3 
		#define returnMove 4
		#define returnStop 5

int currentState = readyState;
const int rotationStep = 48; //Amount of steps required by stepper motor to complete a single step

//define position of items in the array 2D array
#define moveStep 0
#define movetime 1
#define moveDir 2

int moves[3][6] = {
    {64, 272, 736, 512, 960, 1472}, //steps required
    {20, 20, 20, 10, 10, 10}, //time taken
		{0, 0, 1, 1, 1, 0}, //dir 0 - means clockwise
};

int holdGetSteps[6] = {-64,-320,400,896,1872,400}; //computed values for getsteps vriable change
int counter = 0; //Holds the timer for return move
int arrayPos = 0; //Chanage this to change order of firing
int remainingSteps = 0;//updted by the return move to move to next state matcheed with the interrrupt value

void statemachine(){
		
		//switch to alternate between states
        switch (currentState) {
					
					case readyState:
						if (pressed && (arrayPos<=5)) { //prevent overflow of array by limiting 
							pressed = false ; // acknowledge the button press
							currentState = makeMove;
							
							setTimer(0, computePitCounter(moves[moveStep][arrayPos],moves[movetime][arrayPos])) ; 
							
							moveSteps(m1, moves[moveStep][arrayPos], (moves[moveDir][arrayPos] == 1) ? 1 : 0) ; // move 64 steps clockwise
							
							startTimer(0); //interrupt will now start causing stepping of servo motor
						}
					break;
					case makeMove:
						
				
					//if the motor has stopped running i.e no more steps left then move onto complete
					if (getSteps(m1)== holdGetSteps[arrayPos] ) {
                currentState = completeMove ;
          }else if(pressed_stop){
						
						 //if user press the stop button and if the counter is not at 0
							pressed_stop = false ; // acknowledge the button
							stopTimer(0); 
							currentState = moveStop;
						
					}
				
					break;
					case moveStop:
					//if user press the run button
						if (pressed) {
							pressed = false ; // acknowledge
							startTimer(0); //resume the timer to complete
							currentState = makeMove;
						}
					break;
					case completeMove:
						//await for button press to initiate return
						if (pressed) {
							pressed = false ; // acknowledge
							currentState = returnMove;
							originalPosStep = 0 ; //reset value to be counted by the interrupt
							
							//store the number of steps in +ve and -ve, then if -ve modulus it to get a positive value
							
						int temp=0; //define a new temporary value to hold the remaining steps left to move back to original pos
							
							//amount of steps already taken, check if there are any remaining steps to take
							if((moves[moveStep][arrayPos] % rotationStep) == 0){
								arrayPos++;//move to next pos in array
								currentState = readyState; // no more move to make we are at 12 oclock position
								
							}else{
								temp = moves[moveStep][arrayPos] / rotationStep;
								temp = moves[moveStep][arrayPos] - (temp*rotationStep);
								
								if(temp > (rotationStep/2)){ //determine which half the position of the dial is
									
									//to correct for anti-clockwise move by checking the direction
									
									remainingSteps = rotationStep - temp; //update the remaining steps to determine if the counter has reached original position
									
								if(moves[moveDir][arrayPos] == 1){
									moveSteps(m1, rotationStep-temp, true) ;				
								}else{
									moveSteps(m1, rotationStep - temp, false) ; 
								}
						
									counter = 5; //delay counter for return to original position, required to slow down speed of stepper
									currentState = returnMove; // no more move to make we are at 12 oclock position
								}else{
									remainingSteps = temp;
								if(moves[moveDir][arrayPos] == 1){
									moveSteps(m1, temp, false) ; // move steps to original position
								}else{
									moveSteps(m1, temp, true) ; 
								}
									counter=5;
									currentState = returnMove; // no more move to make we are at 12 oclock position
								}
								
							}
						}
						
					break;
					case returnMove:
					
					if (pressed_stop) {
						pressed_stop = false ; // acknowledge
						currentState = returnStop;
						stopTimer(0);
					}else{
			
					//10ms is too fast for the stepper motor move back to original position at fixed  rate
					if (counter == 0) {
						counter = 5 ;
						updateMotor(m1) ;
					} else {
							if (counter > 0) counter-- ;
					}	
					
					if(originalPosStep==remainingSteps){
							//increment to the next move
							arrayPos++; //onto next move
             currentState = readyState; // no more move to make we are at 12 oclock position
          }
					
				}
					
					break;
					case returnStop:
					if (pressed) {
						pressed = false ; // acknowledge
						currentState = returnMove;
						startTimer(0);
						}	
						
					break;
						
        }
			}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/

int main (void) {
    configureGPIOoutput() ; 
    configureGPIOinput() ; //Required to read the buttons for run and stop
    configureMotor() ;
	  configurePIT(0) ;            // Configure PIT channel 0
    Init_SysTick(1000) ; // SysTick every ms
    waitSysTickCounter(10) ; // initialise counter
    
    while (1) {        

			StartPollInput() ;
			StopPollInput();
			statemachine();
      waitSysTickCounter(10) ; // cycle every 10ms
			
    }
}

