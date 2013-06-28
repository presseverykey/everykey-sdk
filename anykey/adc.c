#include "adc.h"

void ADC_Init() {
	/* 2. Power and peripheral clock: In the SYSAHBCLKCTRL register, set bit
	13 (Table 25). Power to the ADC at run-time is controlled through the
	PDRUNCFG register (Table 55). */
  ADC->AD0CR.CLKDIV = 16; /* TODO : this is the value for 72Mhz */
  
  SYSCON->PDRUNCFG      &= (~SYSCON_ADC_PD);
  SYSCON->SYSAHBCLKCTRL |= SYSCON_SYSAHBCLKCTRL_ADC;

}

void ADC_Disable() {
  SYSCON->SYSAHBCLKCTRL &= (~SYSCON_SYSAHBCLKCTRL_ADC);
  SYSCON->PDRUNCFG      |= SYSCON_ADC_PD;
}

/*
  Read a value from the specified channel. This may be done after:
    - the ADC subsystem was activated using: ADC_Init()
    - and the pin corresponding to the ADC channel was set to ADC
      function mode using: GPIO_SETFUNCTION

      pins map to channels as follows:

      Channel Port Pin
      ----------------
      0       0    11 
      1       1     0
      2       1     1
      3       1     2
      4       1     3
      5       1     4
      6       1    10
      7       1    11

     
  Values are read in software mode (not burst), with full (10bit)
  accuracy.
  
  Returns -1 in case the OVERRUN bit was set, indicating the current
  value overwrote a previously unread value. This shouldn't happen.
*/
int32_t ADC_Read(uint8_t channel) {
  ADDR addr;

  ADC->AD0CR.SEL    = 1 << channel;
  ADC->AD0CR.BURST  = ADC_BURST_SW;
  ADC->AD0CR.CLKS   = ADC_CLKS_11C_10B;
  ADC->AD0CR.START  = ADC_START_START;
  
  do {
    addr = ADC->AD0DR[channel];
  } while (!addr.DONE);
  
  ADC->AD0CR.START  = ADC_START_NOSTART;
//  if (addr.OVERRUN) {
//    return -1;
//  }
  return addr.V_VREF;
}
