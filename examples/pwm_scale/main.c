#include "everykey/everykey.h"


// slightly more advanced pwm example. 
// This plays the C major scale on the pwm
// MAT0 pin, advancing one note every time the
// button is pressed.
//
// It demonstrates how to simply calculate the
// values for prescaler and match register for a 
// given frequency.

#define KEY_PORT 0
#define KEY_PIN  1

typedef struct {
	uint16_t pre;
	uint16_t match;
} note;

// notes of the C major scale have the following
// frequencies starting at middle C:
//   C : 261.63
//   D : 293.66
//   E : 329.63
//   F : 349.23
//   G : 392.0
//   A : 440.0
//   B : 493.88
//   C : 523.25
//
// to calculate the require prescaler and match register,
// use the following equation:
// 
// freq = clock / (prescaler * match)
//
// and find the values for prescaler and match that 
// result in freq being as close to the actual frequency
// you are targeting. Included in this directory is
// a small ruby script I've used to brute force the 
// value below:

#define num_notes 7
note notes[] = {
	(note){77,  3574},   // C
	(note){14,  17513},  // D
	(note){33,  6619},
	(note){8,   25771},
	(note){7,   26239},
	(note){4,   40909}, // A
	(note){8,   18223},
	(note){107, 1286},  // C'
};

int currnote = 0;

void play_note(note n) {
	// This function sets the PWM pin up to produce a square
	// wave of the frequency providing by the note.
	Timer_Enable(CT16B0, true);
	// The prescaler (divisor) needs to be reduced by 1, in the
	// formula above the prescaler is the value that the system
	// clock needs to be divided by. In the microcontroller, the
	// prescaler is a counter value, which is incremented on each system
	// clock tick. When the value is reached the timer counter is increased.
	// So a prescale value of 0 is the same as dividing by 1, because the 
	// timer counter is incremented at every system clock tick.
	Timer_SetPrescale(CT16B0, n.pre-1);
	// We're producing a simple square wave, so we need to turn the 
	// pin on half way to the match value.
	Timer_SetMatchValue(CT16B0, 0, n.match >> 1);
	Timer_SetMatchValue(CT16B0, 1, n.match);
	// once the match value is reached, the timer is reset.
	Timer_SetMatchBehaviour(CT16B0, 1, TIMER_MATCH_RESET);
	Timer_EnablePWM(CT16B0, 0, true);
	Timer_Start(CT16B0);
}

void main(void) {
	// use LED for fancy button press indicator!
	every_gpio_set_dir(0,7,OUTPUT);
	
	// configure the button as input.
	every_gpio_set_dir(KEY_PORT, KEY_PIN, INPUT);
	EVERY_GPIO_SET_PULL(KEY_PORT, KEY_PIN, PULL_UP);
	
	// configure the PIN 0_8 to act as timer match output.
	EVERY_GPIO_SET_FUNCTION(0,8,TMR, IOCON_IO_ADMODE_DIGITAL);

	// start playing the first note.
	play_note(notes[currnote]);

	// start the systick timer to check for button presses.
	SYSCON_StartSystick_10ms();

	while(true) {}
}

int button_presses = 0;
void systick() {
	// simple debounce operation. Reading that the button has
	// been pressed three times in a row (30ms) gives us enough 
	// convidence that the button has actually been pressed...
	if (!every_gpio_read(KEY_PORT, KEY_PIN)) {
		if (button_presses == 0xff) {
			return;
		}
		button_presses += 1;
		if (button_presses >= 3) {
			button_presses = 0xff;
		}
		return;
	}	

	if (button_presses == 0xff) {
		// button has been released
		if (every_gpio_read(KEY_PORT, KEY_PIN)) {
			// button has been released
			currnote += 1;
			if (currnote > num_notes) {
				// start from the beginning after finshing the scale.
				currnote = 0;
			}
			// switch light on and off for each press ...
			every_gpio_write(0,7,currnote & 1);
			// play the next note.
			play_note(notes[currnote]);
		}
	}
	button_presses = 0;
}
