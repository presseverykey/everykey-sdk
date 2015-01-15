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

/** set memory */
void* memset(void* b, int c, uint32_t len);

/** copy memory */
void* memcpy(void* to, const void* from, uint32_t len);

/** compare memory */
int memcmp(const void *s1, const void *s2, uint32_t n);

/** determine the length of a C string (not including termination) */
uint32_t strlen(const char* s);

/** copy a C string (including termination) */
char* strcpy(char *dst, const char *src);

#endif


