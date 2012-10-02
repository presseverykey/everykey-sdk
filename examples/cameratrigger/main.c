#include "pressanykey/pressanykey.h"

#define IR_PORT 2
#define IR_PIN 3
#define LED_PORT 0
#define LED_PIN 7
#define KEY_PORT 1
#define KEY_PIN 4
#define MIC_PORT 1
#define MIC_PIN 4
#define OPTOCOUPLER_PORT 2
#define OPTOCOUPLER_PIN 11

const uint16_t threshold = 200;
const uint32_t deaf_timeout = 76800;

typedef enum State {
	Scanning = 0,
	Triggering = 1,
	Deaf = 2
} State;

State state;
uint32_t counter;

uint8_t toggle;

uint16_t min;
uint16_t max;

void main(void) {
	state = Deaf;
	counter = 0;
	
	//	GPIO_SetDir(KEY_PORT, KEY_PIN, GPIO_Input);
	//	GPIO_SETPULL(KEY_PORT, KEY_PIN, IOCON_IO_PULL_UP);
	
	GPIO_SetDir(IR_PORT, IR_PIN, GPIO_Output);
	GPIO_WriteOutput(IR_PORT, IR_PIN, false);
	
	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
	GPIO_WriteOutput(LED_PORT, LED_PIN, false);

	GPIO_SetDir(OPTOCOUPLER_PORT, OPTOCOUPLER_PIN, GPIO_Output);
	GPIO_WriteOutput(OPTOCOUPLER_PORT, OPTOCOUPLER_PIN, false);
	
	GPIO_SetDir(MIC_PORT, MIC_PIN, GPIO_Input);
	ADC_Init();
	GPIO_SETPULL(MIC_PORT, MIC_PIN, IOCON_IO_PULL_NONE);
	GPIO_SETFUNCTION(MIC_PORT, MIC_PIN, ADC, IOCON_IO_ADMODE_ANALOG);

	SYSCON_StartSystick(937);
	
}


void systick_scan() {
	int i = ADC_Read(5);
	if ((counter%2000) == 0) {
		int val = max - min;
		if (val > threshold) {
			GPIO_WriteOutput(LED_PORT, LED_PIN, true);
			GPIO_WriteOutput(OPTOCOUPLER_PORT, OPTOCOUPLER_PIN, true);
			state = Triggering;
			counter = 0;
			return;
		}
		max = 0;
		min = 0x3ff;
	} else {
		if (max<i) max = i;
		if (min>i) min = i;
	}
	counter++;
	GPIO_WriteOutput(LED_PORT, LED_PIN, (counter / 10000) & 1);
}

void systick_trigger() {
	//usec					cycles
	//    0 ..  2000         0 ..  154 on
	//29830 .. 30230      2291 .. 2322 on
	//31810 .. 32210      2443 .. 2474 on
	//35790 .. 36190      2749 .. 2779 on
	
	//63200 .. 65200      4854 .. 5007 on
	//93030 .. 93430      7145 .. 7175 on
	//95010 .. 95410      7297 .. 7327 on
	//98990 .. 99390      7602 .. 7633 on
	
	
	bool on =	(counter >= 0 && counter < 154) ||
				(counter >= 2291 && counter < 2322) ||
				(counter >= 2443 && counter < 2474) ||
				(counter >= 2749 && counter < 2779) ||

				(counter >= 4854 && counter < 5007) ||
				(counter >= 7145 && counter < 7175) ||
				(counter >= 7297 && counter < 7327) ||
				(counter >= 7602 && counter < 7633);

	GPIO_WriteOutput(IR_PORT, IR_PIN, on && ((toggle++) & 1));

	counter++;
	if (counter > 8000) {
		state = Deaf;
		counter = 0;
	}
}

void systick_deaf() {
	counter++;
	if (counter > deaf_timeout) {
		state = Scanning;
		counter = 0;
		max = 0;
		min = 0;
		GPIO_WriteOutput(OPTOCOUPLER_PORT, OPTOCOUPLER_PIN, false);
		GPIO_WriteOutput(LED_PORT, LED_PIN, false);
	}
}

void systick() {
	
	switch (state) {
		case Scanning:
			systick_scan();
			break;
		case Triggering:
			systick_trigger();
			break;
		case Deaf:
			systick_deaf();
			break;
	}
}


