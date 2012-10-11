#include "state.h"

int32_t currentPosition[NUM_AXES];
int32_t stateFlags;
CommandStruct currentCommand;

void State_Init() {
	int i;
	for (i=0; i<NUM_AXES; i++) currentPosition[i] = 0;
	stateFlags = State_SteppersOn | State_ImmediateMode;
}
