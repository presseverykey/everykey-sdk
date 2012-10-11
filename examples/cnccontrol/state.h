/** CNC state */
#ifndef _STATE_
#define _STATE_

#include "pressanykey/pressanykey.h"
#include "cnctypes.h"
#include "config.h"

extern int32_t currentPosition[NUM_AXES];
extern int32_t stateFlags;
extern CommandStruct currentCommand;

void State_Init();


#endif