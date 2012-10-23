/** CNC state */
#ifndef _STATE_
#define _STATE_

#include "pressanykey/pressanykey.h"
#include "cnctypes.h"
#include "config.h"


/** current position of the machine */
extern int32_t currentPosition[NUM_AXES];

/** global state flags, ORed StateFlags values */
extern int32_t stateFlags;

/** the command we're currently working on */
extern CommandStruct currentCommand;

/** number of ticks within the current command */
extern uint32_t currentCommandTicks; 

/** Spindle speed. 0 = no motion, 0xffff = max speed */
uint16_t spindleSpeed;

void State_Init();


#endif