#include "downstream.h"
#include "cnctypes.h"
#include "state.h"
#include "cmdqueue.h"

int ledCounter;
uint16_t spindlePhase;
uint16_t spindleCompare;

void SetEnablePin(uint8_t axis, bool on) {
	if (ENABLE_IS_LOW_ACTIVE) on = !on;
	every_gpio_write(axes[axis].enablePort, axes[axis].enablePin, on); 
}

void SetDirPin(uint8_t axis, bool increasing) {
	every_gpio_write(axes[axis].dirPort, axes[axis].dirPin, increasing); 
}

void SetStepPin(uint8_t axis, bool value) {
	every_gpio_write(axes[axis].stepPort, axes[axis].stepPin, value); 
}

void SetSpindlePin(bool on) {
	if (SPINDLE_IS_LOW_ACTIVE) on = !on;
	every_gpio_write(SPINDLE_PORT, SPINDLE_PIN, on);
}				

/** set the spindleSpeed value and our internal PWM compare value according to spindle speed value (0-65535) */
void SetSpindleSpeed(uint16_t speed) {
	spindleSpeed = speed;
	spindleCompare = (uint16_t)((((uint32_t)speed) * (SPINDLE_PWM_RESOLUTION + 1)) / 0xffff);
}

void Downstream_Init() {
	EVERY_GPIO_SET_FUNCTION(0, 10, PIO, IOCON_IO_ADMODE_DIGITAL);
	EVERY_GPIO_SET_FUNCTION(0, 11, PIO, IOCON_IO_ADMODE_DIGITAL);
	
	int i;
	for (i=0;i<NUM_AXES;i++) {
		
		every_gpio_set_dir(axes[i].dirPort, axes[i].dirPin, OUTPUT);
		SetDirPin(i, false);

		every_gpio_set_dir(axes[i].stepPort, axes[i].stepPin, OUTPUT);
		SetStepPin(i, false);

		every_gpio_set_dir(axes[i].enablePort, axes[i].enablePin, OUTPUT);
		SetEnablePin(i, true);	//Right now, we enable all drivers - should be made dynamic later
	}

	every_gpio_set_dir(SPINDLE_PORT, SPINDLE_PIN, OUTPUT);
	SetSpindlePin(false);
	
	every_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
	every_gpio_write(LED_PORT, LED_PIN, false);

	ledCounter = 0;
	spindlePhase = 0;
	SetSpindleSpeed(0);
}

void Downstream_Tick() {

	int i;
	bool wantNewCommand = false;	//if true after handling our command, we will try to find a new one
	
	//power steppers on
	for (i=0; i<NUM_AXES; i++) {
		SetEnablePin(i, true);
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
		case CMD_SPINDLE_IMM:
			if (currentCommandTicks < currentCommand.args.SPINDLE_IMM.ticks) {	//Ramping speed up or down
				int32_t remaining = currentCommand.args.SPINDLE_IMM.ticks - currentCommandTicks;
				int32_t delta = currentCommand.args.SPINDLE_IMM.speed - spindleSpeed;	
				uint16_t newSpeed = spindleSpeed + (delta/(remaining+1));
				SetSpindleSpeed(newSpeed);
			} else {														//we're at the end
				SetSpindleSpeed(currentCommand.args.SPINDLE_IMM.speed);
				wantNewCommand = true;
			}
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
			break;
		case CMD_WAIT:
			if (currentCommandTicks > currentCommand.args.WAIT.ticks) {
				wantNewCommand = true;
			}
			break;
		case CMD_SPINDLE:
			if (currentCommandTicks < currentCommand.args.SPINDLE.ticks) {	//Ramping speed up or down
				int32_t remaining = currentCommand.args.SPINDLE.ticks - currentCommandTicks;
				int32_t delta = currentCommand.args.SPINDLE.speed - spindleSpeed;	
				uint16_t newSpeed = spindleSpeed + (delta/(remaining+1));
				SetSpindleSpeed(newSpeed);
			} else {														//we're at the end
				SetSpindleSpeed(currentCommand.args.SPINDLE.speed);
				wantNewCommand = true;
			}
			break;
	}
	
	//set dir bits
	for (i=0; i<NUM_AXES; i++) {
		if (currentPosition[i] > lastPosition[i]) {	//increasing movement
			SetDirPin(i, true);
		} else if (currentPosition[i] < lastPosition[i]) { //decreasing movement
			SetDirPin(i, false);
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
	spindlePhase = (spindlePhase + 1) % SPINDLE_PWM_RESOLUTION;
	SetSpindlePin(spindlePhase < spindleCompare);
//	every_gpio_write(LED_PORT, LED_PIN, spindlePhase < spindleCompare);

	//Blink the LED to show we're alive
	ledCounter++;
	every_gpio_write(LED_PORT, LED_PIN, ledCounter & 0x800);
	
	//set step bits
	for (i=0; i<NUM_AXES; i++) {
		SetStepPin(i, (currentPosition[i] >> SUBSTEP_BITS) & 1);
	}
	
}

