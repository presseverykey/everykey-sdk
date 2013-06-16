#include "pressanykey/pressanykey.h"

#define LED_PORT 0
#define LED_PIN 7

//systick call interval in processor cycles. For 72MHz, 2000 will result in 36 kHz
#define SYSTICK_CYCLES 2000

//pwm speed in systick intervals. For 36 kHz systick, 256 will result in roughly 144 Hz.
#define PWM_SPEED 256

//animation speed in systick intervals. For 36 kHz systick, 256 will result in roughly 144 Hz.
#define ANIM_SPEED 256

//triangle wave size, one side in anim speed. With 512 steps (full wave), we'll repeat all 3.5s.
#define ANIM_STEPS 256

uint32_t counter = 0;	//we count cycles

void main(void) {	// main is used for setup and starting the systick timer
	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
	GPIO_WriteOutput(LED_PORT, LED_PIN, false);
	Timer_Enable(CT16B0, true);
	Timer_SetPrescale(CT16B0, 0);				//Use full clock speed
	Timer_SetMatchValue(CT16B0, 0, 2000);
	Timer_SetMatchBehaviour(CT16B0, 0, TIMER_MATCH_INTERRUPT | TIMER_MATCH_RESET);
	NVIC_EnableInterrupt(NVIC_CT16B0);
	Timer_Start(CT16B0);

}

void ct16b0_handler(void) {	//systick is used for regular, repeating tasks
	Timer_ClearInterruptMask(CT16B0);
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
	GPIO_WriteOutput(LED_PORT, LED_PIN, brightness > pwmPhase);
}
