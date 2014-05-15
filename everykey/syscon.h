/***************************************
 System configuration functions
***************************************/

#ifndef _SYSCON_
#define _SYSCON_

/** run CPU with max speed using external oscillator */
void SYSCON_InitCore72MHzFromExternal12MHz();

/** Start systick using system clock
@param cycles 24bit value of clocks to elapse between systick invocations  */
void SYSCON_StartSystick(uint32_t clocks);

// Systick is intended to be a timer that is universal to all
// Cortex M3 processors, with an interrupt interval of 10ms
// see User Manual §17.7
void SYSCON_StartSystick_10ms();

/** Stop systick */
void SYSCON_StopSystick(void);


#endif
