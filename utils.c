#include "utils.h"

void sleepCycles(uint32_t duration) {
	volatile uint32_t counter = 0;
	while (counter < duration) counter++;
}

void sleep() {
	__asm ( "WFI\n" );
}
