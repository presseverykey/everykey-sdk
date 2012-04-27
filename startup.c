#include "anykey.h"

void main(void);	//to be implemented by user
void systick(void);	//to be implemented by user

void bootstrap(void);	//bootstrap that will call main later
void deadend(void);	//neverending handler

// Define the vector table - make sure it's mapped to 0x0.
__attribute__ ((section("vectors")))

const VECTOR_TABLE vtable = {
	(void*)0x10001ff0,	//Stack top
	bootstrap,
	deadend,
	deadend,
	deadend,
	deadend,
	deadend,
	deadend,
	deadend,
	deadend,
	deadend,
	deadend,
	deadend,
	deadend,
	deadend,
	systick
};

void bootstrap(void) {
//TODO: Move define to some better form in memorymap
#define NVIC_CCR ((volatile unsigned long *)(0xE000ED14))
	*NVIC_CCR = *NVIC_CCR | 0x200; // Set STKALIGN in NVIC
	SYSCON_InitCore72MHzFromExternal12MHz();	//let there be speed
	SYSCON->SYSAHBCLKCTRL |= 0x40;	// Enable GPIO clock
	main();
	while (true) {
		sleep();
	}
}

void deadend(void) {
	while(1);
}

