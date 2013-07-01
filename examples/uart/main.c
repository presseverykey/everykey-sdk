#include "anykey/anykey.h"

#define LED_PORT 0
#define LED_PIN 7

//simple wait routine
void delay(int count) {
	volatile int i;
	for (i=0; i<count; i++) {}
}

void main(void) {
	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);

	while (true) {
		GPIO_WriteOutput(LED_PORT, LED_PIN, true);
		delay(1000000);
		GPIO_WriteOutput(LED_PORT, LED_PIN, false);
		delay(1000000);
	}
}
