#include "state.h"

int32_t currentPosition[NUM_AXES];
int32_t stateFlags;
CommandStruct currentCommand;
uint32_t currentCommandTicks; //number of ticks within the current command

void State_Init() {
	int i;
	for (i=0; i<NUM_AXES; i++) currentPosition[i] = 0;
	stateFlags = State_SteppersOn | State_ImmediateMode;
	currentCommand.command = CMD_STOP;
	currentCommandTicks = 0;
}
