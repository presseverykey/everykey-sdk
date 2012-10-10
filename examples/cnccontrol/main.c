#include "pressanykey/pressanykey.h"

#define LED_PORT 0
#define LED_PIN 7

#define SYSTICK_INTERVAL ((72000000/1000)-1) /* 1KHz */

/* pin mapping:
 
 D-SUB pin	LPC Port/pin
 1/C0		1_5	
 2/D0		0_8
 3/D1		0_9
 4/D2		0_1
 5/D3		2_0
 6/D4		1_7
 7/D5		1_6
 8/D6		0_11
 9/D7		0_6
 14/LF		2_11
 16/NIN		2_2
 17/NSP		2_3

 X axis: 300mm (Head along the portal)
 Y axis: 400mm (Portal along the table)
 
Mapping of controller
 Function	D-SUB pin
 X en		14
 X dir		17
 X step		16
 Y en
 Y dir		5
 Y step		4
 Z en
 Z dir		7
 Z step		6
 W en
 W dir
 W step
 Spindle	1

 Other enable ports are 8,9,?

 */

typedef struct Axis {
	uint8_t enablePort;
	uint8_t enablePin;
	uint8_t stepPort;
	uint8_t stepPin;
	uint8_t dirPort;
	uint8_t dirPin;
} Axis;

#define NUM_AXES 3

const Axis axes[4] = {
	{ 2,11,	2,2,	2,3 },
	{ 0,11,	0,1,	2,0 },
	{ 0,6,	1,7,	1,6 }
};

uint32_t counter;

void main(void) {
	GPIO_SETFUNCTION(0, 10, PIO, IOCON_IO_ADMODE_DIGITAL);
	GPIO_SETFUNCTION(0, 11, PIO, IOCON_IO_ADMODE_DIGITAL);

	int i;
	for (i=0;i<NUM_AXES;i++) {
		GPIO_SetDir(axes[i].enablePort, axes[i].enablePin, GPIO_Output);
		GPIO_WriteOutput(axes[i].enablePort, axes[i].enablePin, false);

		GPIO_SetDir(axes[i].stepPort, axes[i].stepPin, GPIO_Output);
		GPIO_WriteOutput(axes[i].stepPort, axes[i].stepPin, false);

		GPIO_SetDir(axes[i].dirPort, axes[i].dirPin, GPIO_Output);
		GPIO_WriteOutput(axes[i].dirPort, axes[i].dirPin, false);
	}
	
	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
	GPIO_WriteOutput(LED_PORT, LED_PIN, false);

	counter = 0;
	
	SYSCON_StartSystick(SYSTICK_INTERVAL);
}


void systick() {
	counter++;
	bool enable = (counter / 2000) & 1;
	bool dir = (counter / 1000) & 1;
	bool step = counter & 1;
	
	int i;
	for (i=0;i<NUM_AXES;i++) {
		GPIO_WriteOutput(axes[i].enablePort, axes[i].enablePin, enable);
		GPIO_WriteOutput(axes[i].stepPort, axes[i].stepPin, step);
		GPIO_WriteOutput(axes[i].dirPort, axes[i].dirPin, dir);
	}
	
	GPIO_WriteOutput(LED_PORT, LED_PIN, enable);

}
