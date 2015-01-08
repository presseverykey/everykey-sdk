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


void* memset(void* b, int c, uint32_t len) {
        uint8_t* buf = b;
        uint32_t i;
        for (i=0; i<len; i++) {
                buf[i] = c;
        }
        return b;
}

void* memcpy(void* to, const void* from, uint32_t len) {
        uint8_t* dst = to;
        const uint8_t* src = from;
        uint32_t i;
        for (i=0; i<len; i++) {
                dst[i] = src[i];
        }
        return to;
}


