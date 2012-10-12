#include "cmdqueue.h"
#include "config.h"
#include "state.h"

static CommandStruct queue[CQ_LENGTH];
//if both indexes are the same, the queue is assumed to be empty.
//Must never be completely full
static int readIdx;		//next index to be read
static int writeIdx;	//next index to be written
static uint32_t lastTransactionId;


void* pressanykeymemmove (void* s1, const void* s2, size_t n) {
	size_t i;
	if (s1 < s2) {
		for (i=0; i<n; i++) {
			((uint8_t*)s1)[i] = ((const uint8_t*)s2)[i];
		}
	} else {
		i = n;
		while (i > 0) {
			i--;
			((uint8_t*)s1)[i] = ((const uint8_t*)s2)[i];
		}
	}
	return s1;
}

void CQ_Init() {
	readIdx = 0;
	writeIdx = 0;
	lastTransactionId = 0;
}

bool CQ_GetCommand() {
	disableInterrupts();
	bool ok = (readIdx != writeIdx);
	if (ok) {
		pressanykeymemmove(&currentCommand, &(queue[readIdx]), sizeof(CommandStruct));
		readIdx = (readIdx+1) % CQ_LENGTH;
	}
	enableInterrupts();
	return ok;
}

bool CQ_AddCommand(CommandStruct* cmd) {
	bool ok = true;
	disableInterrupts();
	if (cmd->command > IMMEDIATE_SEPARATOR) {	//Queued command
		int filled = writeIdx - readIdx;
		if (filled < 0) filled += CQ_LENGTH;
		int avail = CQ_LENGTH - filled - 1;
		if (avail > 0) {
			pressanykeymemmove(&(queue[writeIdx]), cmd, sizeof(CommandStruct));
			writeIdx = (writeIdx+1) % CQ_LENGTH;
		} else ok = false;
	} else {									//immediate command
		pressanykeymemmove(&currentCommand, cmd, sizeof(CommandStruct));
		readIdx = 0;
		writeIdx = 0;
	}
	lastTransactionId = cmd->transactionId;
	enableInterrupts();
	return ok;
}			

bool CQ_ReadAvail() {
	 disableInterrupts();
	 bool avail = (readIdx != writeIdx);
	 enableInterrupts();
	 return avail;
}

int CQ_EmptySlots(uint32_t* outLastTransactionId) {
	disableInterrupts();
	int filled = writeIdx - readIdx;
	if (filled < 0) filled += CQ_LENGTH;
	int avail = CQ_LENGTH - filled - 1;
	if (outLastTransactionId) *outLastTransactionId = lastTransactionId;
	enableInterrupts();
	return avail;
}

void CQ_Clear() {
	disableInterrupts();
	readIdx = 0;
	writeIdx = 0;
	enableInterrupts();
}
