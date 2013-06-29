#include "anykey/anykey.h"

#define LED_PORT 0
#define LED_PIN 7

//simple wait routine
void delay(int count) {
	volatile int i;
	for (i=0; i<count; i++) {}
}

void main(void) {
	any_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);

	while (true) {
		any_gpio_write(LED_PORT, LED_PIN, true);
		delay(1000000);
		any_gpio_write(LED_PORT, LED_PIN, false);
		delay(1000000);
	}
}
