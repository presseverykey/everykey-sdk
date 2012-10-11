#include "downstream.h"
#include "cnctypes.h"
#include "state.h"

int counter;

void SetEnable(uint8_t axis, bool on) {
	if (ENABLE_IS_LOW_ACTIVE) on = !on;
	GPIO_WriteOutput(axes[axis].enablePort, axes[axis].enablePin, on); 
}

void SetDir(uint8_t axis, bool increasing) {
	GPIO_WriteOutput(axes[axis].dirPort, axes[axis].dirPin, increasing); 
}

void SetStep(uint8_t axis, bool value) {
	GPIO_WriteOutput(axes[axis].stepPort, axes[axis].stepPin, value); 
}

void SetSpindle(bool on) {
	if (SPINDLE_IS_LOW_ACTIVE) on = !on;
	GPIO_WriteOutput(SPINDLE_PORT, SPINDLE_PIN, !on);
}				


void Downstream_Init() {
	GPIO_SETFUNCTION(0, 10, PIO, IOCON_IO_ADMODE_DIGITAL);
	GPIO_SETFUNCTION(0, 11, PIO, IOCON_IO_ADMODE_DIGITAL);
	
	int i;
	for (i=0;i<NUM_AXES;i++) {
		
		GPIO_SetDir(axes[i].dirPort, axes[i].dirPin, GPIO_Output);
		SetDir(i, false);

		GPIO_SetDir(axes[i].stepPort, axes[i].stepPin, GPIO_Output);
		SetStep(i, false);

		GPIO_SetDir(axes[i].enablePort, axes[i].enablePin, GPIO_Output);
		SetEnable(i, true);	//Right now, we enable all drivers - should be made dynamic later
	}

	GPIO_SetDir(SPINDLE_PORT, SPINDLE_PIN, GPIO_Output);
	SetSpindle(false);
	
	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
	GPIO_WriteOutput(LED_PORT, LED_PIN, false);

	counter = 0;
}

void Downstream_Tick() {
	int i;

	//power steppers on
	for (i=0; i<NUM_AXES; i++) {
		SetEnable(i, true);
	}
	
	//remember last position
	uint32_t lastPosition[NUM_AXES];
	for (i=0; i<NUM_AXES; i++) lastPosition[i] = currentPosition[i];
	
	//handle command
	switch (currentCommand.command) {
		case CMD_STOP:
			break;
		case CMD_MOVE_DIR:
			for (i=0; i< NUM_AXES; i++) {
				currentPosition[i] += currentCommand.args.MOVE_DIR.delta[i];
			}
			break;
		case CMD_SPINDLE_ON:
			stateFlags |= State_SpindleOn;
			break;
		case CMD_SPINDLE_OFF:
			stateFlags &= ~State_SpindleOn;
			break;
	}
	
	//set dir bits
	for (i=0; i<NUM_AXES; i++) {
		if (currentPosition[i] > lastPosition[i]) {	//increasing movement
			SetDir(i, true);
		} else if (currentPosition[i] < lastPosition[i]) { //decreasing movement
			SetDir(i, false);
		}
	}

	//set spinle (put as much delay as possible between set dir and set step)
	SetSpindle(stateFlags & State_SpindleOn);
	
	//set step bits
	for (i=0; i<NUM_AXES; i++) {
		SetStep(i, (currentPosition[i] >> 8) & 1);
	}
	
	counter++;
	GPIO_WriteOutput(LED_PORT, LED_PIN, counter & 0x800);
/*
	bool enable = (counter / 2000) & 1;
	bool dir = (counter / 1000) & 1;
	bool step = counter & 1;
	
	int i;
	for (i=0;i<NUM_AXES;i++) {
		GPIO_WriteOutput(axes[i].enablePort, axes[i].enablePin, enable);
		GPIO_WriteOutput(axes[i].stepPort, axes[i].stepPin, step);
		GPIO_WriteOutput(axes[i].dirPort, axes[i].dirPin, dir);
	}
*/
	
}

