#include "downstream.h"
#include "cnctypes.h"
#include "state.h"
#include "cmdqueue.h"

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
	GPIO_WriteOutput(SPINDLE_PORT, SPINDLE_PIN, on);
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
	bool wantNewCommand = false;	//if true after handling our command, we will try to find a new one
	
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
			wantNewCommand = true;
			break;
		case CMD_MOVE_DIR:
			for (i=0; i< NUM_AXES; i++) {
				currentPosition[i] += currentCommand.args.MOVE_DIR.delta[i];
			}
			break;
		case CMD_SET_HOME:
			for (i=0; i< NUM_AXES; i++) {
				currentPosition[i] = 0;
			}
			wantNewCommand = true;
			break;
		case CMD_SPINDLE_ON:
			stateFlags |= State_SpindleOn;
			wantNewCommand = true;
			break;
		case CMD_SPINDLE_OFF:
			stateFlags &= ~State_SpindleOn;
			wantNewCommand = true;
			break;
		case CMD_MOVE_TO_IMM:
		{
			bool arrived = true;
			for (i=0; i< NUM_AXES; i++) {
				int32_t delta = currentCommand.args.MOVE_TO_IMM.target[i] - currentPosition[i];
				if (delta) {
					arrived = false;
					bool neg = delta < 0;
					if (neg) delta = -delta;
					if (delta > currentCommand.args.MOVE_TO_IMM.speed) delta = currentCommand.args.MOVE_TO_IMM.speed;
					if (neg) delta = -delta;
					currentPosition[i] += delta;
				}
			}
			if (arrived) {
				wantNewCommand = true;
			}
		}
			break;
		case CMD_MOVE_TO:
			if (currentCommandTicks < currentCommand.args.MOVE_TO.ticks) {	//Moving on path
				int32_t remaining = currentCommand.args.MOVE_TO.ticks - currentCommandTicks;
				for (i=0; i<NUM_AXES; i++) {	//make sure we're exactly on target
					int32_t delta = currentCommand.args.MOVE_TO.target[i] - currentPosition[i];
					currentPosition[i] += delta / (remaining+1);
				}
			} else {														//we're at the end
				for (i=0; i<NUM_AXES; i++) {	//make sure we're exactly on target
					currentPosition[i] = currentCommand.args.MOVE_TO.target[i];
				}
				wantNewCommand = true;
			}
			
			//TODO
			break;
		case CMD_WAIT:
			if (currentCommandTicks > currentCommand.args.WAIT.ticks) {
				wantNewCommand = true;
			}
			break;
		case CMD_SPINDLE_ON_SCRIPT:
			stateFlags |= State_SpindleOn;
			wantNewCommand = true;
			break;
		case CMD_SPINDLE_OFF_SCRIPT:
			stateFlags &= ~State_SpindleOn;
			wantNewCommand = true;
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
			
	//put as much delay as possible between set dir and set step - do everything we can here
	
	//update current command ticks
	currentCommandTicks++;
				
	//poll new command if needed
	if (wantNewCommand) {
		if (!(CQ_GetCommand())) currentCommand.command = CMD_STOP;
		currentCommandTicks = 0;
	}

	//update immediate mode flag
	if (currentCommand.command > IMMEDIATE_SEPARATOR) stateFlags |= State_ImmediateMode;
	else stateFlags &= ~State_ImmediateMode;
	
	//set spindle
	SetSpindle(stateFlags & State_SpindleOn);

	//Blink the LED to show we're alive
	counter++;
	GPIO_WriteOutput(LED_PORT, LED_PIN, counter & 0x800);
	
	//set step bits
	for (i=0; i<NUM_AXES; i++) {
		SetStep(i, (currentPosition[i] >> SUBSTEP_BITS) & 1);
	}
	
}

