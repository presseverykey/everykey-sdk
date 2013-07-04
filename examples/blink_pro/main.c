
// This a more "real life" version of the embedded "Hello World": making
// an LED blink. If you are entirely new to embedded programming, have a
// look at the beginner's version in the `examples/blink` directory
// first. The comments in here assume you've read through that example.

// First off, we're not taking the easy route to handle Pin IO and will
// configure the LED pin manually ... We no longer need to include the
// anypio.h header, instead, we need to directly include the "low level"
// SDK:

#include "anykey/anykey.h"

// To use the sdk in your own projects, the anykey directory needs to be
// copied or linked to your project directoy.

// In the microcontroller, physical pins are organized into "ports" and
// "pins". To be clean, we define the values for the LED port and pin
// here:

#define LED_PORT 0
#define LED_PIN  7

// We'll come back to IO in a little bit.

// The main problem with the beginner's example was that we just wasted
// time between turning the LED on and off. We used a large `for` loop:
//
//  	for (i=0; i<count; i++) {}
// 
// to keep the processor busy between switching the LED. This is fine if
// we only want to switch the LED on and off, but in case we also want
// to do something useful, it would be nice for the processor to be
// available for work in between switching.
//
// In this example we will use Timers and Interrupt-Routines to make the
// LED blink. Timers and interrupts are central concepts in embedded
// programming, so we'd like to introduce them early:
//
// Timers are basically what the name suggests: they are initialized to
// a certain time period and once that period elapses they generate an
// event (this sound wishy washy and it is: we'll elaborate a little
// later..) Think of it like a tea timer: you set it to 5 minutes and
// after that time it rings. The important bit is that the timer keeps
// you from looking at your watch every couple of seconds and frees you
// up to do more important things during those 5 minutes. Analogously, a
// timer in an embedded system doesn't occupy the CPU freeing it to do
// more important things.
//
// Interrupts are functions that can be triggered to run by the
// processor at any time. There are a lot of events that can cause an
// interrupt routine to trigger, in our case the event triggering the
// interrupt will be an elapsing timer. 
//
// The useful thing about interrupts is that they be triggered at any
// time, even when the processor is busy doing other things. Before
// running the interrupt routine, the processor saves the work it's
// currently doing and once the interrupt routine has finished
// executing, the processor can pick up it's original work right where
// it left off.

void main(void) {
	// We're back at IO: since a pin can be used a number of different
	// pursposes. We need to configure the pin to be used for pin IO:

	ANY_GPIO_SET_FUNCTION(LED_PORT, LED_PIN, PIO, IOCON_IO_ADMODE_DIGITAL);

	// (strictly speaking, this last step wasn't necessary for the LED
	// because port 0, pin 7 is set up for pin IO per default, but some
	// other pins aren't, so it's a good habit to explicitly configure the
	// pin function. Else, in the furture you will use a pin that is not
	// set to IO per default an spend hours wondering why nothing is
	// working...)
	
	// Finally, pins configured for pin IO can either be OUTPUT (we write
	// a value to them) or INPUT we read the value physically present at
	// the connected pin. We need to set the pin configuration to output
	// mode, because we want to turn the LED on and off:

	any_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);

	// We defined LED_PORT and LED_PIN to identify the LED in the code
	// above. OUTPUT is a constant of the type any_gpio_direction and
	// defined in anykey/gpio.h which was included indirectly by
	// including: anykey/anykey.h ...

	// Next we need to configure the timer and interrupt. All Cortex M3
	// processors include a "systick" timer, which, when activated
	// triggers the "systick" interrupt to run every 10ms.

	SYSCON_StartSystick_10ms();
}

// The LPC1343 (and many other processors) require an interrupt table
// that keeps track of where the functions that run when an interrupt is
// triggered are located. This table is placed at the very beginning of
// the firmware and it's defined in the file: anykey/startup.c. 
//
// If you have a look at the interrupt table, you'll see the first entry
// refers to the top of the stack (ignore that for now) and the second
// entry refers to `bootstrap`. Once the processor is ready to run, an
// event is generated that triggers this bootstrap interrupt routine to
// run. `bootstrap` does a little bit of initialization and then calls
// your `main` function. So if you ever wondering "who calls main", now
// you know.
//
// The interrupt table also contains a `systick` routine which gets
// called for each systick event. This routine is preconfigured to do
// nothing, so we'll just write a new version to blink:

void systick(void) {

	// Like stated above, this interrupt routine is run once every 10ms.
	// This is much to fast to blink the LED. We want it to blink every
	// second for a tenth of a second, so we'll need to count:

	static uint32_t counter = 0;

	// the `static` modifier causes this variable to be reused every time
	// the function is called and maintain it's value. Else, we would get
	// a fresh new counter set to 0 every time the routine is called.

	// We'll increment the counter by 2 every time it's called:
	counter += 2;

	// This is just a trick to remember whether to turn the LED on or off:
	
	if (counter == 20) {
		// `counter` reaches 20 after systick has executed 10 times, so after
		// a tenth of a second (10 * 10ms (10/1000s) = 1/10s). Once a tenth
		// of a second has elapsed, we turn the LED off.
		any_gpio_write(LED_PORT, LED_PIN, false);
		
		// here's the trick: we don't reset the counter to 0, but to 1.
		// Since we're always incrementing by 2, we'll now that if the
		// counter has an odd value, we are waiting to turning the LED on.
		// Likewise, once the LED is turned on, we reset the counter to 0
		// and will now that we're waiting to turn it back off because the
		// counter value is even.

		counter = 1;
	} else if (counter == 181) {
		// the counter reaches 181 after 90/100th of a second, because we
		// increment by 2 every 1/100th of a second and start counting at 1.
		// This has the effect that the LED will blink once a second for
		// 1/10s...

		any_gpio_write(LED_PORT, LED_PIN, true);

		// reset counter to 0 in order to remeber to turn it back off!
		counter = 0;
	}
}


