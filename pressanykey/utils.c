#include "utils.h"


void waitForInterrupt() {
	__asm ( "WFI\n" );
}
