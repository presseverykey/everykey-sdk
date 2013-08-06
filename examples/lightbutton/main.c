#include "anypio.h"

// This is a simple example of how to interact with the buttons on the
// Anykey. 
// A somewhat more elaborate example (using interrupts) is contained int
// the lightbutton_pro directory.


void main(void) {

	// ! in case you are lucky (?) enough to own
	// ! a vintage first edition anykey: the second
	// ! edition got an extra button and the pins are
	// ! mapped differently, so you'll need to replace
	// ! KEY1_REV2 with KEY_REV1 in the code below.

	// The buttons on the Ankey are conntect to ground when they are
	// pushed, but when they aren't pushed they're not connected to
	// anything. (They're said to be "floating"). If we read from a button
	// that's floating our the value we read is more or less random. 
	// We need to find a way to ensure we read a logical 1 when the button
	// is not pressed and a 0 when it's pressed.
	//
	// To do this, a so called "Pull-up" Resistor is introduced:
	//
	//  
	//             Switch     Pull-up
	//
	// Ground _______/ --+-----NNN---- V+
	//                   |
	//                  -+-
	//                  PIN
	//
	// When the switch is not pressed, the value read at the pin will be
	// V+, because that's the only thing connected. The pull-up resistor
	// has a very large resistance, so that when the switch is closed, the
	// circuit will be completed from ground to the pin and that we avoid
	// having a short circuit from Ground to V+.
	//
	// Fortunately, we don't actually have to attach any resistors to
	// the switches, because the LPC1334 has built-in pull-up resistors
	// that we can activate from our firmware. 
	//
	// Incidentally, pull-downs are analogous to pull ups except
	// everything is turned around: if you have a switch, that, when
	// pressed connects to V+ instead of ground, you need to a pull down
	// resistor so you reliably read a logical 0 when the switch is not
	// pressed. PULL_DOWN resistors can also be configured in the
	// firmware.
	//
	// In order to configure the pin to use a pull-up resistor, we do the
	// following:

	anypio_digital_input_set(KEY1_REV2, PULL_UP);

	while (true) {
		// now read the button.
		bool button = anypio_read(KEY1_REV2);

		// write the value read from the button to the LED. Remember that
		// the buttons are connected to ground , so you will
		// read a logical 0 when the button is pressed.
		
		anypio_write(LED, button);
	}
}


