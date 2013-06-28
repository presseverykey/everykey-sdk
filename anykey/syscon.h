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

/** Stop systick */
void SYSCON_StopSystick(void);


#endif
