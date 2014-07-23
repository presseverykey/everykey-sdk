
#include "pwmaudio.h"

void PWMAudio_Init() {
	every_gpio_set_dir(PWMAUDIO_PORT, PWMAUDIO_PIN, OUTPUT);
	EVERY_GPIO_SET_FUNCTION(PWMAUDIO_PORT, PWMAUDIO_PIN, TMR, ADMODE_DIGITAL); 

	Timer_Enable(CT16B0, true);
	Timer_SetPrescale(CT16B0, 0);					//Use full clock speed

	Timer_SetMatchBehaviour(CT16B0, 0, 0);
	Timer_SetMatchBehaviour(CT16B0, 1, 0);
	Timer_SetMatchBehaviour(CT16B0, 2, 0);
	Timer_SetMatchValue(CT16B0, 3, 512);			//Set fixed PWM speed
	Timer_SetMatchBehaviour(CT16B0, 3, TIMER_MATCH_RESET);

	Timer_EnablePWM(CT16B0, PWMAUDIO_MAT, true);	//Turn on PWM
	Timer_SetMatchValue(CT16B0, PWMAUDIO_MAT, 128);

	Timer_Start(CT16B0);
}

void PWMAudio_SetSample(int16_t sample) {
	Timer_SetMatchValue(CT16B0, PWMAUDIO_MAT, (sample + 0x8000) >> 7) ;		//set PWM
}
