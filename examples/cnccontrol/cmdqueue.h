#ifndef _CMDQUEUE_
#define _CMDQUEUE_

#include "anykey/anykey.h"
#include "cnctypes.h"
#include "config.h"

/** initialize queue state */
void CQ_Init();


/** Copies the next command to currentCommand, if available. Leaves currentCommand as is otherwise.
 @return success */
bool CQ_GetCommand();

/** adds a command that came in. If it's an immediate command, it is copied directly to 
 currentCommand and the queue is cleared. Otherwise, it's put onto the queue. */
bool CQ_AddCommand(CommandStruct* cmd);
	
/** returns whether there's a command to read or not */
bool CQ_ReadAvail();

/** returns the number of empty slots */
int CQ_EmptySlots(uint32_t* outLastTransactionID);

/** clears the queue */
void CQ_Clear();

#endif

