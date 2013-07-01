// This is a test program to make sure that all pins work and that none
// of them are short circuited.
// Upon loading the program and resetting the key, all gpio pins of the
// processor that are connected to pads along the edge of the board are
// high.
// Pressing the key will turn on one pad after the next in numerical
// order:
//
//           26     (...)     14
//				+---------------------+
//				|O                    |
//			1	|                     |
//	(...)	|                    +---
//				|     TOP            | U
//				|                    | S
//				|    +----+          | B
//				|    |    |          +---
//				|    |Proc|           |
//		 13 |    +----+           |
//				+---------------------+
//
// Pads are "numbered" counterclockwise, starting at the left (looking a
// the key form the top, i.e processor, side so left pads (top to
// bottom) are 1 - 13 and the top pads (from left to right) 14-26

// MOST of the pads are connected to GPIO pins, with the following
// exceptions:
//
// Pad
//  5				Open drain
// 12				Open drain
// 17       External Power or USB Power
// 18       Regulated 3.3V used as ADC reference
// 19       Ground
//
// So ... Pads 5 and 12 WON'T light up (if you are testing like we do
// with a LED connected to GND on the cathode and trying the pads with
// the anode) .  They will be low (draining) when off.
//
// Finally, Pad 20 is connected to the key, it will be turn to high
// after pad 26, of course afterwards the key will no longer work and
// you'll need to reset it to test the key again.

#include "anykey/anykey.h"


#define KEY_PORT 1
#define KEY_PIN 4

#define LED_PORT 0
#define LED_PIN 7

# define SYSTICK_CNTR 0x000AFC7F



// port and pin are stuck together in on byte ...
const uint8_t pinports[] = {
	0x17, // 1_7
	0x16, // 1_6
	0x01, // 1_0
	0x20, // 2_0
	0x04,
	0x10,      
	0x06,
	0x0B,
	0x0A,
	0x02,
	0x07,
	0x05,
	0x21,

	0x22,
	0x11,
	0x12,
	//0, // ext bpower
	//0, // vdd
	//0, // gnd
	//(1<<4 & 4), // // button special case. pad 20/19
	0x23,
	0x13,
	0x15,
	0x2B,
	0x09,
	0x08
};




#define NUM_PIN_PORT 22
void set_functions() {
	// some pads aren't in PIO mode per default,
	// i.e. PIO1_0, pad 6, proc pin 33
	// set all of them.
	ANY_GPIO_SET_FUNCTION(1,7,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(1,6,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(0,1,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(2,0,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(0,4,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(1,0,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(0,6,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(0,11,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(0,10,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(0,2,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(0,7,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(0,5,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(2,1,PIO,IOCON_IO_ADMODE_DIGITAL)

	ANY_GPIO_SET_FUNCTION(2,2,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(1,1,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(1,2,PIO,IOCON_IO_ADMODE_DIGITAL)
	// pwr
	// vdd
	// gnd
	// key
	ANY_GPIO_SET_FUNCTION(2,3,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(1,3,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(1,5,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(2,11,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(0,9,PIO,IOCON_IO_ADMODE_DIGITAL)
	ANY_GPIO_SET_FUNCTION(0,8,PIO,IOCON_IO_ADMODE_DIGITAL)
};

void initialize_ports (void) {
	int i;
	uint8_t pp, pin, port;
	set_functions();
	// turn all pads on ...
	for (i=0; i!=NUM_PIN_PORT; ++i) {
		pp = pinports[i];
		
		// split entry into port and pin halves
		port = 0x0f & (pp>>4);
		pin  = 0x0f & pp;
		
		any_gpio_set_dir(port,pin, OUTPUT);
		any_gpio_write(port, pin, true);
	}
	// ... except the key pad
	any_gpio_set_dir(KEY_PORT, KEY_PIN, INPUT);
	ANY_GPIO_SET_PULL(KEY_PORT, KEY_PIN, IOCON_IO_PULL_UP);
}

// turn on pad number `pad+1`, the pad parameter is
// an offset into the `pinports` array.
void set_pad(int * pad) {
	
	uint8_t i, port, pin;
	uint8_t pp; 
	bool val;

	for (i = 0; i!= NUM_PIN_PORT; ++i) {
		pp = pinports[i];
		// split apart port/pin info
		port = 0x0f & (pp >> 4);
		pin  = 0x0f & pp;

		if (i == *pad) {
			val = true;	    // turn on the current pad
		} else {
			val = false;	  // and everything else off
		}
		any_gpio_write(port, pin, val);
	}
	
	// once we've reached the last pad, turn the the
	// pad connected to the button on. (pad 20)
	if (*pad == NUM_PIN_PORT) {
		any_gpio_set_dir(KEY_PORT, KEY_PIN, OUTPUT);
		any_gpio_write(KEY_PORT, KEY_PIN, true);
	} 
	
	*pad += 1;
	
}

// key_state keeps track of whether button was pushed or not the last
// time we checked.
bool key_state;
// `pad` keeps track of the pad we're currently testing.
int pad;
#define pressed false

// systick is run once every 10ms ...
void systick(void) {
	bool curr_state;	
	
	// check whether the button is pressed ...
	curr_state = any_gpio_read(KEY_PORT, KEY_PIN);

	if (key_state != curr_state) {
		if (curr_state == pressed) {
			// light the led when pressed
			any_gpio_write(LED_PORT, LED_PIN, true);
		} else {
			// and then turn it back off when releasing...
			any_gpio_write(LED_PORT, LED_PIN, false);
			// ... and finally light the next pad.
			set_pad(&pad);
		}
	}
	key_state = curr_state;
}

void main (void) {
	key_state = true;
	pad = 0;
	initialize_ports();
  SYSCON_StartSystick(SYSTICK_CNTR);
}

