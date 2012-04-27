#ifndef _UTILS_
#define _UTILS_

#include "types.h"

/** simple, rather unprecise C sleep method.
 @param duration time to sleep, each unit is roughly 6-7 cycles (depending on compiler and settings)
 */
void sleepCycles(uint32_t duration);

/** puts the CPU to sleep until an interrupt occurs */
void sleep();


#endif


