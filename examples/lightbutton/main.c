#include "anykey/anykey.h"

#define LED_PORT 0
#define LED_PIN 7
#define KEY_PORT 0
#define KEY_PIN 1

void main(void) {
	any_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
	any_gpio_set_dir(KEY_PORT, KEY_PIN, INPUT);
	ANY_GPIO_SET_PULL(KEY_PORT, KEY_PIN, IOCON_IO_PULL_UP);

	while (true) {
		bool button = any_gpio_read(KEY_PORT, KEY_PIN);
		any_gpio_write(LED_PORT, LED_PIN, button);
	}
}
