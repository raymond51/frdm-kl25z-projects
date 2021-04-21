#include <MKL25Z4.H>
// Function prototypes for cycle timing using SysTick

extern volatile uint32_t ButtonCounter1ms ;

void Init_SysTick(uint32_t ticksPerSec) ;
void waitSysTickCounter(int ticks) ;
