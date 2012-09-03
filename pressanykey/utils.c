#include "utils.h"


void waitForInterrupt() {
	__asm ( "WFI\n" );
}

void* memcpy(void* s1, const void* s2, size_t n) {
	size_t i;
	for (i=0; i<n; i++) {
		((uint8_t*)s1)[i] = ((uint8_t*)s2)[i];
	}
	return s1;
}
