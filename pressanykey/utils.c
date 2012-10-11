#include "utils.h"


void waitForInterrupt() {
	__asm ( "WFI\n" );
}

void disableInterrupts() {
	__asm (
		 "MOV R0, #1\n"
		 "MSR PRIMASK, R0\n"
		 :
		 :
		 : "r0"
	);
}

void enableInterrupts() {
	__asm (
		 "MOV R0, #0\n"
		 "MSR PRIMASK, R0\n"
		 :
		 :
		 : "r0"
	);
}

