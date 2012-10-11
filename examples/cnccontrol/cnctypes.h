/** our custom types */

#ifndef _CNCTYPES_
#define _CNCTYPES_

#include "pressanykey/pressanykey.h"

#define NUM_AXES 3

typedef struct Axis {
	uint8_t enablePort;
	uint8_t enablePin;
	uint8_t stepPort;
	uint8_t stepPin;
	uint8_t dirPort;
	uint8_t dirPin;
} Axis;

typedef enum StateFlags {
	State_SteppersOn	= 0x01,
	State_SpindleOn		= 0x02,
	State_ImmediateMode = 0x04,
	State_EmergencyOff	= 0x08,
} StateFlags;	
	
typedef enum CommandId {
	CMD_STOP				= 0,	//needs to be 0 - start state
	CMD_MOVE_DIR			= 1,
	CMD_MOVE_TO				= 2,
	CMD_SET_HOME			= 3,
	CMD_SPINDLE_ON			= 4,
	CMD_SPINDLE_OFF			= 5
} CommandId;

typedef struct CommandStruct {
	uint32_t command;
	union {
		struct {
			int16_t delta[NUM_AXES];
		} MOVE_DIR;
		struct {
			int32_t target[NUM_AXES];
			uint32_t ticks;
		} MOVE_TO;
	} args;
} __attribute__((packed)) CommandStruct;


typedef struct ResponseStruct {
	uint32_t currentPos[NUM_AXES];
	uint32_t stateFlags;
} ResponseStruct;

#endif
