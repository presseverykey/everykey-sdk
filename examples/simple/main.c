#include "anykey/anykey.h"


//simple wait routine
void delay(int count) {
	volatile int i;
	for (i=0; i<count; i++) {}
}

void main(void) {

	any_gpio_set_dir(LED, OUTPUT);

	while (true) {
		any_gpio_write(LED, true);
		delay(1000000);
		any_gpio_write(LED, false);
		delay(1000000);
	}
}
