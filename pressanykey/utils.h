#ifndef _UTILS_
#define _UTILS_

#define NOP { __asm volatile ( "NOP\n"); }

#include "types.h"

/** puts the CPU to sleep until an interrupt occurs */
void waitForInterrupt();

/** temporarily disable interrupts (for atomic access - only do this for a short time) */
void disableInterrupts();

/** re-enable interrupts */
void enableInterrupts();

#endif


