// This example program is the "Hello World" of embedded programming:
// a blinking LED.

// The first thing we need to do is include header files for our
// `pio` library (Pin Input/Output). We will be turning on and off
// the LED by alternatingly providing power to a pin and turning the pin
// off again. In between, we wait.

#include "everypio.h"

// Turning a pin on or off is about the simplest thing you can do on an
// embedded device, but it does require a bit of boilerplate that can be
// daunting at first. The `everypio` library takes care of all that for
// you.
//
// (In case you want to use everypio for your own projecti later, you need
// to copy or link it into your project directory. The source is located
// under `libs/everypio`)

// first we declare a delay function to allow us to wait in between
// turning the LED off and on. The implementation is at the bottom of
// the file, but because C compilers read source strictly from top to
// bottom, the compiler needs to know about a functions existance before
// it can be used.

void delay(int count);

// The main function is the entry point into our program. This is not
// the first function called when the system starts, though. Before the
// system can run, some intialization needs to be taken care of. If you
// are interested in what happens when the system boots up, have a look
// at: 
//       everykey/startup.c
void main(void) {
	// the program is just an infinite loop:
	while (true) {
		everypio_write(LED, true);
		delay(1000000);
		everypio_write(LED, false);
		delay(1000000);
	}
}

//simple wait routine
void delay(int count) {

	// this delay function basically just keeps the processor busy
	// counting for a while.
	//
	// The `volatile` modifier to the `i` variable is very important at
	// this point. Modern compilers will be able to tell that the loop
	// counting `i` basically does nothing and will optimize it away.  The
	// `volatile` modifier tells the compiler that the variable can change
	// "at will", i.e. the value stored in `i` may not be the value that
	// was written there by the program. *

	volatile int i; 
	for (i=0; i<count; i++) {} 

	// This is obviously not a very efficient way to wait for a little
	// while. The processor will be entirely occupied by just counting and
	// would not have any time at all to do any actual work.
	//
	// See the `blink_pro` example for a more efficient way to make the
	// LED blink using a timer and interrupts. This example also shows how
	// Pin IO is configured explicitly.
}


// * regarding `volatile` it may see strange at first that a variable
// could change it's value without any intervention of the executing
// program. One example how this can happen is that IO pins are mapped
// to predefined memory locations on embedded systems. The value of a
// memory location is dependant on whether something is physically
// connected to a pin or not. The value of a variable refering to this
// memory location would depend on the physical state of the entire
// system and is entirely indepedant of the running program. Of course,
// in the `delay` function, we just (ab)use this behaviour to make the
// processor actually waste time counting ...
