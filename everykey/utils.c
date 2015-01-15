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

int memcmp(const void *s1, const void *s2, uint32_t n) {
    uint32_t i;
    const uint8_t* buf1 = s1;
    const uint8_t* buf2 = s2;
    for (i=0; i<n; i++) {
        if (buf1[i] != buf2[i]) {
            return buf2[i] - buf1[i];
        }
    }
    return 0;
}

uint32_t strlen(const char* s) {
    uint32_t i = 0;
    while (s[i]) {
        i++;
    }
    return i;
}

char* strcpy(char *dst, const char *src) {
    memcpy(dst, src, strlen(src) + 1); //copy including termination
}


