
#include "anykey/anykey.h"



void main(void) {

	any_gpio_set_dir(0,7,OUTPUT);
	ANY_GPIO_SET_FUNCTION(0,8,TMR, IOCON_IO_ADMODE_DIGITAL);
	ANY_GPIO_SET_FUNCTION(0,9,TMR, IOCON_IO_ADMODE_DIGITAL);
	
	Timer_Enable(CT16B0, true);
	Timer_SetPrescale(CT16B0, 720);       // divider for the system clock: 72MHz 100kHz
																				// PWM pins start LOW and will switch to HIGH when
																				// the value in the timer reaches the value
																				// in their match register is reached
	Timer_SetMatchValue(CT16B0, 0, 1000); // set match register for 1st PWM, switch HIGH after 1/100 s
	Timer_SetMatchValue(CT16B0, 1, 2000); // set match register for 2nd PWM, switch HIGH after 1/50  s
	Timer_SetMatchValue(CT16B0, 2, 4000); // the third match register is configure to trigger after 1/25 s ...
	Timer_SetMatchBehaviour(CT16B0, 2, TIMER_MATCH_RESET); // ... and upon being reached will reset the timer
	                                                       // thereby reseting the PWM PINs to LOW.
	

//	first PWM:
//       ------------      ------------
//      |            |    |            |
//      |            |    |            |
//	----              ----              ----
//	|--- 4/100 s ----|
//	    |-- 3/100s --|
//
//	second PWM: (25hz square wave)
//           --------          -------
//          |        |        |       |
//          |        |        |       |
//  --------			    --------         --------
//  |--- 4/100 s ----|
//  |-1/50s-|


	Timer_EnablePWM(CT16B0, 0, true); // enable the PWM for the two channels ...
	Timer_EnablePWM(CT16B0, 1, true);

	Timer_Start(CT16B0); // and start the timer ...

	while(true) {}
}
