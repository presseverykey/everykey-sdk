#include "adc.h"

void ADC_Init() {
	/* 2. Power and peripheral clock: In the SYSAHBCLKCTRL register, set bit
	13 (Table 25). Power to the ADC at run-time is controlled through the
	PDRUNCFG register (Table 55). */
  
  SYSCON->SYSAHBCLKCTRL |= SYSCON_SYSAHBCLKCTRL_ADC;
  SYSCON->PDRUNCFG      &= (~SYSCON_ADC_PD);
}

void ADC_Disable() {

  SYSCON->SYSAHBCLKCTRL &= (~SYSCON_SYSAHBCLKCTRL_ADC);
  SYSCON->PDRUNCFG      |= SYSCON_ADC_PD;
}

