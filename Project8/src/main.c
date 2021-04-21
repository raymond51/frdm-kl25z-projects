/* ------------------------------------------
Project: ESLab 8 - Tune selector
             
This program enables the user to create a tune of 12 corrosponding musical notes,
each note can be configured with different duration. 

Initially the screen prompts the user to select the note to be played and then once selected prompts for duration of play.
Once entered the note selected will be displayed in the row under the first prompt, this will cause a cycle of tone when the left button is pressed.
  -------------------------------------------- */

#include <MKL25Z4.h>
#include "../include/gpio_defs.h"
#include "../include/SysTick.h"
#include "../include/LCD.h"
#include "../include/pit.h"
#include "../include/adc_defs.h"
#include <stdbool.h>

#define BUTTONOPEN (0)
#define BUTTONCLOSED (1)
#define BUTTONBOUNCE (2)


/*----------------------------------------------------------------------------
  GPIO Input Configuration

  Initialse a Port D pin as an input, with no interrupt
  Bit number given by BUTTON_POS
 *----------------------------------------------------------------------------*/ 
void configureGPIOinput(void) {
    SIM->SCGC5 |=  SIM_SCGC5_PORTD_MASK; /* enable clock for port D */

    /* Select GPIO and enable pull-up resistors and no interrupts */
    PORTD->PCR[BUTTON1_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0);
    PORTD->PCR[BUTTON2_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_IRQC(0x0);
    
    /* Set port D switch bit to inputs */
    PTD->PDDR &= ~(MASK(BUTTON1_POS) | MASK(BUTTON2_POS));
}

/* ----------------------------------------
     Configure GPIO output for Audio
       1. Enable clock to GPIO port
       2. Enable GPIO port
       3. Set GPIO direction to output
       4. Ensure output low
 * ---------------------------------------- */
void configureGPIOoutput(void) {
    // Enable clock to port A
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    
    // Make pin GPIO
    PORTA->PCR[AUDIO_POS] &= ~PORT_PCR_MUX_MASK;          
    PORTA->PCR[AUDIO_POS] |= PORT_PCR_MUX(1);          
    
    // Set ports to outputs
    PTA->PDDR |= MASK(AUDIO_POS);

    // Turn off output
    PTA->PCOR = MASK(AUDIO_POS);
} 



/*----------------------------------------------------------------------------
Poll the adc channel to detect a button change
 
*----------------------------------------------------------------------------*/
volatile float measured_voltage ;  // scaled value

float noButtonPress = 3.3; //reads high V when no button pressed
float tolerance = 0.04; //provide acceptable range for v reading
//left,up,down,right
float button_voltage[4] = {2,0.53,1.26,0.003}; // button voltage reading 
int b_state= BUTTONOPEN ; //as only 1 button can be read using adc at a time
//left,up,down,right
int pressed[4] = {0, 0, 0, 0} ;  // signal for corrosponding buttons
int buttonPressedSignal = -1; //no button actively pressed initially
int bounceCounter = 0 ;

void ADCButtonDebounce()  // 0 or 1
{
	  if (bounceCounter > 0) bounceCounter -- ;
	
    		MeasureVoltage() ;    // updates sres variable
				// scale to an actual voltage, assuming VREF accurate
				measured_voltage = (VREF * sres) / ADCRANGE ;
	
    switch (b_state) {
        case BUTTONOPEN :
							//if voltage reading is less than 3.3v we have a button press
            if (measured_voltage<(noButtonPress-tolerance)) {
							
							
							 b_state = BUTTONCLOSED ; //assume button press
							
							//left
							if(measured_voltage >= (button_voltage[0] - tolerance) && measured_voltage <= (button_voltage[0] + tolerance)){
								pressed[0] = 1 ;  // create a 'pressed' event
								buttonPressedSignal = 0;
								//up
							}else if(measured_voltage >= (button_voltage[1] - tolerance) && measured_voltage <= (button_voltage[1] + tolerance)){
								pressed[1] = 1 ;
								buttonPressedSignal = 1;
								//down
							}else if(measured_voltage >= (button_voltage[2] - tolerance) && measured_voltage <= (button_voltage[2] + tolerance)){
								pressed[2] = 1 ;
								buttonPressedSignal = 2;
								//right
							}else if(measured_voltage >= (button_voltage[3] - tolerance) && measured_voltage <= (button_voltage[3] + tolerance)){
								pressed[3] = 1 ;
								buttonPressedSignal = 3;
							}else{
							b_state = BUTTONOPEN ; //no button was pressed within the range therefore reset
							buttonPressedSignal = -1;
							}
							
            }
            break ;
        case BUTTONCLOSED :
					//if no button pressed
            if (measured_voltage>(noButtonPress-tolerance)){
                b_state = BUTTONBOUNCE ;
                bounceCounter= 20 ; //delay for 200ms
            }
            break ;
        case BUTTONBOUNCE :
            if (measured_voltage >= (button_voltage[buttonPressedSignal] - tolerance) && measured_voltage <= (button_voltage[buttonPressedSignal] + tolerance)) {
                b_state = BUTTONCLOSED ;
            }
            if (bounceCounter == 0) {
                b_state = BUTTONOPEN ;
								buttonPressedSignal = -1; //set no button press
            }
            break ;
    }
}

         
/* ----------------------------------------
     Toggle the Audio Output 
 * ---------------------------------------- */
void audioToggle(void) {
    PTA->PTOR = MASK(AUDIO_POS) ;
}

#define NOTE_SCREEN (0)
#define DURATION_SCREEN (1)
#define toneOnDuration 0
#define toneOffDuration 1

int playTune = 0; //initially no tune to play

int currentState = NOTE_SCREEN;

char Notes[16] = { "CcDdEFfGgAaB" } ;
char durationSetting[16] = { "1248" } ;
char noteHistory[16] = {""}; //initially no notes chosen, list will build up

int noteHistoryCount = 0; //Holds the amount of notes played, initially zero

//int durationTime = 0; //must be in count
int durationTime[16]; //must be in count

int cursorPos = 0; //holds the cursor position when traversing with up and down button

//notes array
const uint32_t midiNotes[] = {20040,18915,17853,16851,15905,15013,14170,13375,12624,11916,11247,10616};
//Tone contorl
int tonePlayState = toneOffDuration;
int timeCounterTone = 0; //changing variable which holds the changing time counter
int tuneCounter = 0;

/*----------------------------------------------------------------------------
@breif: Displays the first row of the notes to select from
*----------------------------------------------------------------------------*/
void displayNoteScreen(){

  lcdMode(M_Inc) ; // reset entry mode
 // lcdClear(false) ;  // clear memory
	setLCDAddress(0,0) ;
  writeLCDString(Notes) ;
		lcdHome(true) ;  // reset cursor and shift
	  lcdCntrl(C_BLINK) ; // reset cursor
}
/*----------------------------------------------------------------------------
@breif: Displays the second row of the note history
*----------------------------------------------------------------------------*/
void displayNoteSelectionRow(){
	setLCDAddress(1,0) ;
  writeLCDString(noteHistory) ;
	lcdHome(true) ;  // reset cursor and shift
}

/*----------------------------------------------------------------------------
@breif: Displays the first row of the note duration playtime
*----------------------------------------------------------------------------*/
void displayDurationSelection(){
	
  lcdMode(M_Inc) ; // reset entry mode
  lcdClear(true) ;  // clear memory
	setLCDAddress(0,0) ;
  writeLCDString(durationSetting) ;
	lcdHome(true) ;  // reset cursor and shift
	lcdCntrl(C_BLINK) ; // reset cursor
}

/*----------------------------------------------------------------------------
@breif: State machine
*----------------------------------------------------------------------------*/
void statemachine(){

	switch(currentState){
	
		case  NOTE_SCREEN:
			
			//left - toggle play tune
			if(pressed[0]){
				pressed[0] = 0; //acknowledge press
				
				playTune = !playTune; //toggle play tune
				
				//up - move cursor left
			} else if(pressed[1]){
				pressed[1] = 0;
				cursorShift(D_Left); //Command to move cursor
				cursorPos --; //match the cursor position
				
				//down - move cursor right
			} else if(pressed[2]){
				pressed[2] = 0;
				cursorShift(D_Right);
				cursorPos ++; //match the cursor position
				
				//right - make a selection
			} else if(pressed[3]){
				pressed[3] = 0;
				
				noteHistory[noteHistoryCount] = Notes[cursorPos]; //Append the selection of note in form of char to  note history
		
				playTune = 0; //disable playing tune
				
				currentState=DURATION_SCREEN;//change to the duration selection screen
				displayDurationSelection(); //display the duration selection screen
				cursorPos = 0; //reset the position of the cursor for this state
			}
		break;
		
		case  DURATION_SCREEN:
			
		//up - move cursor left
			if(pressed[1]){
				pressed[1] = 0;
				cursorShift(D_Left);
				cursorPos --;//update cursor position
				
				//down - move cursor right
			} else if(pressed[2]){
				pressed[2] = 0;
				cursorShift(D_Right);
				cursorPos ++;
				
				//right - make a selection
			} else if(pressed[3]){
				pressed[3] = 0;
				
				//set the duration of the tone
				switch(cursorPos){
					case 0: //1sec
						durationTime[noteHistoryCount] = 100; //set the duration of the
					break;
					case 1: //2sec
						durationTime[noteHistoryCount] = 200;
					break;
					case 2: //4sec
						durationTime[noteHistoryCount] = 400;
					break;
					case 3: //8sec
						durationTime[noteHistoryCount] = 800;
					break;
				}
				
				
				noteHistoryCount++;//next position of history will be empty to append a new notes
				cursorPos = 0; // reset cursor position tracking
				currentState=NOTE_SCREEN;//change to the duration selection screen
				displayNoteScreen(); //display the 
				displayNoteSelectionRow();
			}	
		break;
	
	}

}

/*----------------------------------------------------------------------------
@breif: Performs a tempo, cycles through the tune with varying tone length
*----------------------------------------------------------------------------*/
void playTuneFunc(){
			
			if(playTune){
			if(timeCounterTone > 0) timeCounterTone -- ;
				
			setTimer(0, midiNotes[tuneCounter]) ; //update the PIT peripheral Val based on note selection
				//reset the position to form a cycle tune
				if(tuneCounter>noteHistoryCount){
					tuneCounter=0;
				}
				
				switch(tonePlayState){
				case toneOnDuration:
						if(timeCounterTone==0){
						stopTimer(0) ; 
						tonePlayState = toneOffDuration;
						timeCounterTone = durationTime[tuneCounter]/4; // set the tempo duration
						tuneCounter++;
						}
						
						break;
				case toneOffDuration:
						
					if(timeCounterTone==0){
						startTimer(0) ;
						tonePlayState = toneOnDuration;
						timeCounterTone = durationTime[tuneCounter];
						}
				break;
		}
			}else{
				stopTimer(0) ;
			}
}

/*----------------------------------------------------------------------------
  MAIN function
  Configure and then run tasks every 10ms
 *----------------------------------------------------------------------------*/

int main (void) {
		uint8_t calibrationFailed ; // zero expected
	//GPIO INPUT
    configureGPIOinput() ;
		configureGPIOoutput(); //audio output
		//TIMER CONFIG
		configurePIT(0) ; // Configure PIT channel 0
	
    Init_SysTick(1000) ; 
    initLCD() ;
		//ADC peripheral
	  Init_ADC() ;
    calibrationFailed = ADC_Cal(ADC0) ; // calibrate the ADC 
    while (calibrationFailed) ; // block progress if calibration failed
    Init_ADC() ;
    waitSysTickCounter(10) ; // initialise counter
    
		//initial display screen
		displayNoteScreen();
	
    while (1) {        
        ADCButtonDebounce() ; 			
				statemachine();
				playTuneFunc();
        waitSysTickCounter(10) ; // cycle every 10 ms
    }
}

