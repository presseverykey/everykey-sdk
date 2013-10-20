/* PS/2 protocol command layer. This is the middle layer, on top of ps2bus. It implements basic command transfers including resend, return codes,
 device-initiated transfers and basic response handling. */

#ifndef _PS2_CMD_
#define _PS2_CMD_

#include "ps2bus.h"

typedef enum {
    PS2_CMD_COMPLETION_OK,      //Command completed successfully
    PS2_CMD_COMPLETION_TIMEOUT, //Command timed out
    PS2_CMD_COMPLETION_FAIL     //Command failed (unexpected return data or max errors in underlying layers exceeded)
} PS2_CMD_COMPLETIONCODE;


typedef void(*ps2cmd_CommandCompletionHandler)(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);
typedef void(*ps2cmd_DataReceivedHandler)(uint8_t data);
typedef void(*ps2cmd_IdleHandler)();

void ps2cmd_init(ps2cmd_DataReceivedHandler dataReceivedHandler, ps2cmd_IdleHandler idleHandler);

bool ps2cmd_sendCommand(uint8_t command, uint8_t argLength, uint8_t* args, uint8_t expectedResponseLen, ps2cmd_CommandCompletionHandler completion);

bool ps2cmd_commandRunning();

#endif
