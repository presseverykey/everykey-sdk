#include "wdt.h"



void WDT_Start(SYSCON_WDTOSCCTRL_BITS baseFreq, uint8_t divider, uint32_t reload, bool doReset) {
	SYSCON->SYSAHBCLKCTRL |= SYSCON_SYSAHBCLKCTRL_WDT;
 	SYSCON->PDRUNCFG &= ~(SYSCON_WDTOSC_PD);	//always enable watchdog oscillator
	SYSCON->WDTOSCCTRL = baseFreq | divider;	//set freq
	SYSCON->WDTCLKSEL = SYSCON_WDTCLKSEL_WATCHDOG;
	SYSCON->WDTCLKUEN = 0;   //accept clock change
	SYSCON->WDTCLKUEN = 1;
	while (!(SYSCON->WDTCLKUEN & 1)) {}

	SYSCON->WDTCLKDIV = 1;	//enable WDTCLK but don't divide
	WDT->TC = reload;
	WDT->MOD = WDT_MOD_WDEN | (doReset ? WDT_MOD_WDRESET : 0);
} 

/*! Init watchdog to reset with a feed given in microseconds
@param us timeout in microseconds, 1..16777215 */
void WDT_Start_Reset_us(uint32_t us) {
	WDT_Start(SYSCON_WDTOSCCTRL_FREQSEL_4000KHZ, 1, us, true);
}

/*! Feed the watchdog, preventing timeout */
void WDT_Feed() {
	WDT->FEED = WDT_FEED_1;
	WDT->FEED = WDT_FEED_2;
}
