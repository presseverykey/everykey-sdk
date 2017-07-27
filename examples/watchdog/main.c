/*! simple watchdog demo.
- Stats watchdog
- then runs led true for a short time (feeding WDT)
- then runs led false for a short time (feeding WDT)
- then stops feeding the WDT. The MCU should reset
*/
#include "everykey/everykey.h"

#define LED_PORT 0
#define LED_PIN 7

void main(void) {
	every_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
	WDT_Start_Reset_us(100000);
	for (int i=0; i<1000000; i++) {
		every_gpio_write(LED_PORT, LED_PIN, true);
		WDT_Feed();
	}
	for (int i=0; i<1000000; i++) {
		every_gpio_write(LED_PORT, LED_PIN, false);
		WDT_Feed();
	}
	while (1) {}
}
