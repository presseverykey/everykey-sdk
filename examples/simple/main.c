#include "anypio.h"


//simple wait routine
void delay(int count) {
	volatile int i;
	for (i=0; i<count; i++) {}
}

void main(void) {
	while (true) {
		anypio_write(LED, true);
		delay(1000000);
		anypio_write(LED, false);
		delay(1000000);
	}
}
