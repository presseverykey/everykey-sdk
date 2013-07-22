#include "anykey/anykey.h"

// This is a more elaborate, "real-life" example of how to read the value
// of a button (or, for that matter, anything that you may have connected
// externally to a pin).
//
// We're not using the anypio library anymore, butif you like, you can
// mix and match. Since we're not using anypio, we first need to define
// the PIN and PORT numbers corresponding to the physical pins on the
// microprocessor. For convenience, we'll use the buttons and the led,
// but you can use any of the processors pins.

#define LED_PORT 0
#define LED_PIN 7

#define INT1_PORT 0
#define INT1_PIN  1

#define INT2_PORT 0
#define INT2_PIN 0

// Just like in the `blink` and `blink_pro` examples, the reading of the
// button in the basic example in `lightbutton` just read the value of
// the button in an infinite loop. This means the processor is busy with
// only reading the button and there is no time left to anything else.
// It's kind of a wasteful way to deal with processor resources.
// Instead, we will configure an interrupt to trigger whenever the
// button is pressed. Interrupts are code routines tied to certain
// events happening in the system. In our case, the routine is triggered
// whenever the button is pressed. 
//
// Another type of "event" would be a timer running out, or a software
// error such as division by 0 or trying to access an illegal portion of
// memory.

void main(void) {	
	

	// first some boilerplat: we need to configure each of the PINs we'll
	// use to be set to pin i/o (PIO) and to be in digital mode (as
	// opposed to using the A/D conversion for the pin)
	ANY_GPIO_SET_FUNCTION(LED_PORT, LED_PIN, PIO, IOCON_IO_ADMODE_DIGITAL);

	ANY_GPIO_SET_FUNCTION(INT1_PORT, INT1_PIN, PIO, IOCON_IO_ADMODE_DIGITAL);
	ANY_GPIO_SET_FUNCTION(INT2_PORT, INT2_PIN, PIO, IOCON_IO_ADMODE_DIGITAL);

	// more boilerplate: the LED pin should be configured as OUTPUT and
	// we'll turn it on for the start...
	any_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
	any_gpio_write(LED_PORT, LED_PIN, true);

	// The two button pins should be configured as INPUT because we want
	// to read from them. Since the buttons are connected to ground when
	// pushed, we configure them with an internal PULL_UP. If you're
	// uncertain about the purpose of the pull-up, go back and read the
	// `light_button` example.
	any_gpio_set_dir(INT1_PORT, INT1_PIN, INPUT);
	ANY_GPIO_SET_PULL(INT1_PORT, INT1_PIN, PULL_UP);
	any_gpio_set_dir(INT2_PORT, INT2_PIN, INPUT);
	ANY_GPIO_SET_PULL(INT2_PORT, INT2_PIN, PULL_UP);

	// IO Interrupts can be configured to be triggered on a number of
	// events: in our case, we are looking for the RISING and FALLING
	// edges of a signal appilied to the pin. What does that mean?
	//
	// 3.3V ----------+     +---------     <- High Level
	//     Falling -> |     |  <-Rising edge
	//                |     |
	// 0V             +-----+  <-Low Level
	//
	//                ^
	//                |
	//                Button pressed, connected to ground
	//
	// Whenever the button is pressed, the signal read at the pin will
	// transition from HIGH (~3.3V) read through the pull-up to a LOW
	// value (Ground). This transition is called an EDGE. A transition
	// from HIGH to ground is called a falling edge while a transition
	// from LOW to HIGH is called a rising edge. (The IO interrupts can
	// also be configured to trigger at either HIGH or LOW level...)
	//
	// In order to demostrate the differences, the two buttons are
	// configured for different types of EDGE interrupts. Button 1 (the
	// one in the middle of the Anykey) is configured to trigger the
	// interrupt on a falling edge. Since a falling edge is a transition
	// from high to low, and pressing the button will connect the pin to
	// ground, button one triggers the interrupt when it's pressed.
	//
	// Button 2 (the one on the side of the key) triggers the interrupt on
	// a rising edge. This means that the interrupt is triggered when the
	// button is released.

	any_gpio_set_interrupt_mode(INT1_PORT, INT1_PIN, TRIGGER_FALLING_EDGE);	
	any_gpio_set_interrupt_mode(INT2_PORT, INT2_PIN, TRIGGER_RISING_EDGE);	

	// Only to more steps: we need to tell the processor to activate the
	// interrupt ...
	NVIC_EnableInterrupt(NVIC_PIO_0);
	// This activates the interrupt for ALL of port 0, since both of
	// our button pins are grouped in this port, we only have to write a
	// single routine.
}

// When the IO interrupt is triggered, the processor looks for a
// function called `gpio<PORT>_handler` and runs that. (Have a look at
// `anykey/startup.c` to see where these function names are defined.)

void gpio0_handler(void) {
	// because this handler function is called for ALL interrupts trigger
	// by PORT 0, we need a way to find out which pin actually triggered
	// it. 
	uint32_t mask = any_gpio_get_interrupt_mask(INT1_PORT);

	// the bits set in this mask correspond to the pin that triggered the
	// interrupt. In this example, we won't actually use the value,
	// so we'll go ahead and clear the mask:
	any_gpio_clear_interrupt_mask(INT1_PORT, mask);

	// Finally, we toggle the value of the LED.
	bool val = any_gpio_read(LED_PORT, LED_PIN);
	any_gpio_write(LED_PORT, LED_PIN, !val);

	// In this example, we're not actually using the mask, but try
	// this excercise: change the handler function to only toggle the LED
	// after both buttons have been pressed. 
	//
	// HINT: don't reset the mask after each interrupt...
} 
