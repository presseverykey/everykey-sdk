#include "anykey/anykey.h"

#define LED_PORT 0
#define LED_PIN 7

//systick call interval in processor cycles. For 72MHz, 2000 will result in 36 kHz
#define SYSTICK_CYCLES 2000

//pwm speed in systick intervals. For 36 kHz systick, 256 will result in roughly 144 Hz.
#define PWM_SPEED 256

//animation speed in systick intervals. For 36 kHz systick, 256 will result in roughly 144 Hz.
#define ANIM_SPEED 256

//triangle wave size, one side in anim speed. With 512 steps (full wave), we'll repeat every 3.5s.
#define ANIM_STEPS 256

uint32_t counter;	//we count systicks for deriving pwm phase and brightness

void main(void) {	// main is used for setup and starting the systick timer
	any_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
	counter = 0;			//use main to initialize globals, don't init them statically
	SYSCON_StartSystick(SYSTICK_CYCLES-1);	//systick() is called all SYSTICK_CYCLES cycles
}

void systick(void) {	//systick is used for regular, repeating tasks
	counter++;

	// calculate a smooth triangle ramp from counter
	uint32_t brightness = counter / ANIM_SPEED;
	if (brightness >= 2*ANIM_STEPS) counter = 0;	//reset counter
	else if (brightness >= ANIM_STEPS) brightness = 2*ANIM_STEPS-brightness;

	// scale brightness to match pwm
	brightness = (brightness * PWM_SPEED) / ANIM_STEPS;

	// this is the pwm high frequency counter
	uint32_t pwmPhase = counter % PWM_SPEED;

	// set output
	any_gpio_write(LED_PORT, LED_PIN, brightness > pwmPhase);
}
