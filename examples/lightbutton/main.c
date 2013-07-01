#include "anypio.h"


void main(void) {

	// in case you are lucky (?) enough to own
	// a vintage first edition anykey: the second
	// edition got an extra button and the pins are
	// mapped differently, so you'll need to replace
	// KEY1_REV2 with KEY_REV1 in the code below.

	anypio_digital_input_set(KEY1_REV2, PULL_UP);
	while (true) {
		bool button = anypio_read(KEY1_REV2);
		anypio_write(LED, button);
	}
}
