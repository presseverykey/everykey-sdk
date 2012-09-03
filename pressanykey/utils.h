#ifndef _UTILS_
#define _UTILS_

#define NOP { __asm volatile ( "NOP\n"); }

#include "types.h"

/** puts the CPU to sleep until an interrupt occurs */
void waitForInterrupt();

/** very lame memcpy implementation */
void* memcpy(void* s1, const void* s2, size_t n);

#endif


