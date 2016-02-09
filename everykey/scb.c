#include "scb.h"
#include "types.h"
#include "memorymap.h"

void SCB_SystemReset() {
	SCB->AIRCR = AIRCR_VECTKEY | AIRCR_SYSRESETREQ;
}
