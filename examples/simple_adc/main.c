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
  any_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
  
  ADC_Init();
  ANY_GPIO_SET_PULL(ADC_PORT, ADC_PIN, PULL_UP);
  ANY_GPIO_SET_FUNCTION(ADC_PORT, ADC_PIN, ADC, IOCON_IO_ADMODE_ANALOG);

    any_gpio_write(LED_PORT, LED_PIN, true);
    ADC_Read(5);
	while (true) {
		int i = ADC_Read(5);
    //int i = 512;
    any_gpio_write(LED_PORT, LED_PIN, false);
		delay(50 * (i+100));
		any_gpio_write(LED_PORT, LED_PIN, true);
    delay(50 * (1124-i));
	}
  return 0;
}
