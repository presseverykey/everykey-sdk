#include "types.h"
#include "memorymap.h"
#include "syscon.h"

void SYSCON_InitCore72MHzFromExternal12MHz() {
	SYSCON->PDRUNCFG &= ~(SYSCON_SYSOSC_PD | SYSCON_SYSPLL_PD);	//Turn on system oscillator and sys PLL
	SYSCON->SYSPLLCLKSEL = 1;	//choose external oscillator for sys PLL
	SYSCON->SYSPLLCLKUEN = 0;	//Trigger sys pll source change
	SYSCON->SYSPLLCLKUEN = 1;
	SYSCON->SYSPLLCTRL = 0b0100101;	//Fin=12MHz, Fout=72MHz -> M=6, P=2 -> fbdiv=5, postdiv=1
	while (! (SYSCON->SYSPLLSTAT & 1)) {};	//wait for PLL to lock
	SYSCON->MAINCLKSEL = 3;		//Main clock: Sys PLL out
	SYSCON->MAINCLKUEN = 0;		//Trigger main clock source change
	SYSCON->MAINCLKUEN = 1;
}

void SYSCON_StartSystick(uint32_t clocks) {
	SYSCON->SYSTICKCLKDIV = 1;
	SYSTICK->CTRL = 0;
	SYSTICK->LOAD = clocks;
	SYSTICK->VAL = 0;
	SYSTICK->CTRL = 7;
}

void SYSCON_StopSystick(void) {
	SYSTICK->CTRL = 0;
}
