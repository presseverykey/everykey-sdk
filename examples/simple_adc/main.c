#include "anykey/anykey.h"

#define LED_PORT 0
#define LED_PIN  7
#define ADC_PORT 1
#define ADC_PIN  4

//simple wait routine
void delay(int count) {
	volatile int i;
	for (i=0; i<count; i++) {}
}

int main(void) {
  GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
  
  ADC_Init();
  GPIO_SETPULL(ADC_PORT, ADC_PIN, IOCON_IO_PULL_UP);
  GPIO_SETFUNCTION(ADC_PORT, ADC_PIN, ADC, IOCON_IO_ADMODE_ANALOG);

    GPIO_WriteOutput(LED_PORT, LED_PIN, true);
    ADC_Read(5);
	while (true) {
		int i = ADC_Read(5);
    //int i = 512;
    GPIO_WriteOutput(LED_PORT, LED_PIN, false);
		delay(50 * (i+100));
		GPIO_WriteOutput(LED_PORT, LED_PIN, true);
    delay(50 * (1124-i));
	}
  return 0;
}
