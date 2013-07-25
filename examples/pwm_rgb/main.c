#include "anypio.h"


// This is an example of creating a rainbow animation with an RGB LED.
// You'll probably want to read the examples in `pwm` and `pwm_scale`
// because we won't explain the basic of PWM and timers in the example.


/*
2_1  ( 1)       -
0_5  ( 2 - OD)  - 
0_7  ( 3)       - 
0_2  ( 4)       - 
0_10 ( 5)       - B
0_11 ( 6)       - 
0_6  ( 7)       - 
1_0  ( 8)       -
0_4  ( 9 - OD)  - 
2_0  (10)       - 
0_1  (11)       -
1_6  (12)       -
1_7  (13)       -
0_8  (14)       - R
0_9  (15)       - G
2_11 (16)       -
1_5  (17)       -
1_3  (18)       -
2_3  (19)       - 
1_4  (20)       -
1_2  (24)       -
1_1  (25)       - Button
2_2  (26)       - 
*/



// Some defines to keep track of where the connections are located.

#define BUTTON PIN_1_1
#define R PIN_0_8
#define G PIN_0_9
#define B PIN_0_10

void main(void) {
	anypio_write(LED, false);

	anypio_digital_input_set(BUTTON, PULL_DOWN);
	
	// prepare the RGB outputs for PWM (timer and digital mode)
	ANY_GPIO_SET_FUNCTION(0,8,TMR,IOCON_IO_ADMODE_DIGITAL);
	ANY_GPIO_SET_FUNCTION(0,9,TMR,IOCON_IO_ADMODE_DIGITAL);
	ANY_GPIO_SET_FUNCTION(0,10,TMR,IOCON_IO_ADMODE_DIGITAL);

	// enable the timer
	Timer_Enable(CT16B0, true);

	// And set the match value to "about half"
	Timer_SetMatchValue(CT16B0, 0, 32000);
	Timer_SetMatchBehaviour(CT16B0, 0, 0);
	// enable PWM on red pin ...
	Timer_EnablePWM(CT16B0, 0, true);

	// repeat for green ...
	Timer_SetMatchValue(CT16B0, 1, 32000);
	Timer_SetMatchBehaviour(CT16B0, 1, 0);
	Timer_EnablePWM(CT16B0, 1, true);

	// ... and blue
	Timer_SetMatchValue(CT16B0, 2, 32000);
	Timer_SetMatchBehaviour(CT16B0, 2, 0);
	Timer_EnablePWM(CT16B0, 2, true);

	// finally, provide an additional matchvalue to reset the timer to 0 when
	// it reach 65635. Not technically necessary, because the timer's
	// maximum value is 65635 anyway and it would wrap back to 0 by
	// itself.

	Timer_SetMatchValue(CT16B0, 3, 65535);
	Timer_SetMatchBehaviour(CT16B0, 3, TIMER_MATCH_RESET);
	
	// now start the timer!
	Timer_Start(CT16B0);
	
	// There's a `systick` routine below that updates the rainbow
	// animation every 10ms, activate this routine:
	SYSCON_StartSystick_10ms();

}

// define a type to keep track of our RGB color values:
typedef struct {
	uint16_t r;
	uint16_t g;
	uint16_t b;
} rgb;

// And configure an arrary defining the transitions.
// Our rainbow consists of 12 transitions ...
// The format of these color codes is similiar to HTML (#FF00FF) style
// color codes, only that we use 16 bits for each color channel. *

rgb rgbValues[] = {
	{0xffff, 0, 0},      // starting at all red ...
	{0xffff, 0, 0},      // transition to red ... (stay red for one cycle)
	{0xffff, 0xffff, 0}, // transition to yellow ...
	{0xffff, 0xffff, 0}, // stay yellow one cycle ... etc.
	{0, 0xffff, 0},
	{0, 0xffff, 0},
	{0, 0xffff, 0xffff},
	{0, 0xffff, 0xffff},
	{0, 0, 0xffff},
	{0, 0, 0xffff},
	{0xffff, 0, 0xffff},
	{0xffff, 0, 0xffff},
};
// * this explanation is not quite correct for the sake of simplicity,
// we'll explain below ...

uint32_t counter = 0;
void systick() {
	// The counter keeps track of where we are in the rainbow...

	counter++; 

	// The counter value is incremented every 10ms. And we update the
	// actual state of the transistion every tick.  But we'd the
	// transition itself (e.g.from RED to YELLOW) to take longer than
	// 10ms.  
	// `counter` is used to keep track of which transition is
	// currently active (e.g. RED to YELLOW) and the state of the current
	// transistion (e.g. "still nearly RED", or "almost YELLOW")
	// simultaneously.  The bottom six bits of counter keep track of where
	// we are within the transition (the phase).  The top 26 bits keep
	// track of the transition.
	//
	//     0000 0000  0000 0000  0000 0000  0000 0000
	//     TTTT TTTT  TTTT TTTT  TTTT TTTT  TTPP PPPP
	//
	// We'll toggle the LED everytime we change to a new transition. Since
	// the binary mask of the first transition bit is 0x40:
	//
	//     0000 0000  0000 0000  0000 0000  0000 0000
	//     TTTT TTTT  TTTT TTTT  TTTT TTTT  TTPP PPPP
	//     0000 0000  0000 0000  0000 0000  0100 0000
	//
	// and it will toggle every transition, we can use this mask to turn
	// the LED on and off at each subsequent transition.

	anypio_write(LED, counter & 0x40);

	// Since this routine is called every ten ms and the counter needs to
	// reach 0x40 (= 64 decimal) for a new transition to start, each
	// transition takes 64 * 10ms or just over half a second.
	
	// Now we need to figure out the current values for FROM and TO of the
	// transition, to access the values in the transition array above.

	int from = (counter >> 6) % 12;

	// Since we bottom six bits indicate where we are WITHIN the
	// transition we shift them out, leaving us with the value of the
	// transition. Because we only have 12 transitions, but the transition
	// portion of the counter can count to 67108863 (`counter` is 32 bits
	// long, 6 bits are for the phase, leaves 26 bits -> 2**26-1 = 67108863
	// we need to map values 0...67108863 to the range 0..11.
	// To do this, we use the modulo operator ( '%' remainder after division)
	
	int to = (from + 1) % 12;

	// Typically, we'll be transitioning from one color in the 
	// array to the next one. But in the twelfth step, `from` is set to
	// 11. `to` would be 12, but there's only 11 elements in our
	// transition array. Therefore, we need to use the same modulus trick
	// as above to make sure all values keep rolling through...

	// Now that we know WHICH transition we are in, we need to figure out
	// at what step within the transistion we are, the phase...
	//
	// As mentioned, the bottom six bits of `counter` keep track of this.
	// These range from: 00 0000 to 11 1111.
	// The values are basically the porportion of the 'to' color to the
	// 'from' color. At the beginning of the cycle we use 0 of the 'to'
	// color, the amount 'to' color increases each time `systick` is
	// called until, at the end of the cycle, we're using 0 of the 'from'
	// color. 
	// 
	
	uint32_t weight_to   = counter & 0x3f;
  uint32_t weight_from = 0x3F - weight_to;

	// The mask 0x3F corresponds to "all lower 6 bits set" (0011 1111)
	// and, if applied to `counter` with a logical AND, isolates the phase
	// within our transition cycle.

	uint32_t r = (weight_from * rgbValues[from].r + weight_to * rgbValues[to].r ) >> 6;
	uint32_t g = (weight_from * rgbValues[from].g + weight_to * rgbValues[to].g ) >> 6;
	uint32_t b = (weight_from * rgbValues[from].b + weight_to * rgbValues[to].b ) >> 6;

	// Since `proportion_from` and `proportion_to` always add up to
	// 0x3F (0011 1111), we need to divide that factor out again.
	// Shifting by 6 ( `>> 6` ) is equivalent to dividing by 2 ** 6
	// (which is equals to 0x3F) 

	
	// Finally, set the match value of the PWM. If you're not sure what
	// this means, review the `pwm` and `pwm_scale` examples...
	//
	// Our timer-counter is configured to run from 0..0xffff (~66000).
	// Whenever it reaches the match value, it switches the pin connected
	// to it ON. Our RGB values also have the range of 0..0xffff, so
	// setting the match value sets the proportion of time that the pin is
	// turned off. You may have noticed that we resorted to lying above to
	// keep the explaination of the RGB values more simple, the value:
	//
	//    	{0xffff, 0, 0}
	//
	// is not actually red like we said. Setting the red match register to 0xffff means
	// that it's never turned on, and conversely, the green and blue match
	// registers are always on, so this color actually corresponds to
	// HTML color #00ffff (beautiful cyan), the color code for pure red
	// is:
	//  
	//    	{0x0, 0xffff, 0xffff}
	//

	Timer_SetMatchValue(CT16B0, 0, r);
	Timer_SetMatchValue(CT16B0, 1, g);
	Timer_SetMatchValue(CT16B0, 2, b);
}
