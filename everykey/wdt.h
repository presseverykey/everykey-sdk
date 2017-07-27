/***************************************
 Watchdog timer
***************************************/

#ifndef _WDT_H_
#define _WDT_H_

#include "types.h"
#include "memorymap.h"


/*! init and start watchdog with a base frequency, clock divider, reload value and reset or interrupt.
@param baseFreq base frequency - one of the SYSCON_WDTOSCCTRL_FREQSEL constants
@param divider frequency divider, 0..31. 0->div 2, 1->div 4, 2->div 6 ... 31->div 64
@param reload reload value. 24 bits (0..16777215) 
@param doReset if true, a reset is forced. If false, only a WDT interrupt is generated on timeout */
void WDT_Start(SYSCON_WDTOSCCTRL_BITS baseFreq, uint8_t divider, uint32_t reload, bool doReset);

/*! Init watchdog to reset with a feed given in microseconds
@param us timeout in microseconds, 1..16777215 */
void WDT_Start_Reset_us(uint32_t us);

/*! Feed the watchdog, preventing timeout */
void WDT_Feed();

#endif
