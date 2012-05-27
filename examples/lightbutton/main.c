#include "pressanykey/pressanykey.h"

#define LED_PORT 0
#define LED_PIN 7
#define KEY_PORT 1
#define KEY_PIN 4

void main(void) {
	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
	GPIO_SetDir(KEY_PORT, KEY_PIN, GPIO_Input);
	GPIO_SETPULL(KEY_PORT, KEY_PIN, IOCON_IO_PULL_UP);

	while (true) {
		bool button = GPIO_ReadInput(KEY_PORT, KEY_PIN);
		GPIO_WriteOutput(LED_PORT, LED_PIN, button);
	}
}
