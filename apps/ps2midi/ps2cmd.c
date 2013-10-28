#include "ps2cmd.h"
#include "ps2bus.h"

#define PS2_CMD_MAX_ERROR_COUNT 3
#define PS2_CMD_MAX_IDLE_COUNT 5

#define PS2_CMD_MAX_COMMAND_LEN 4
#define PS2_CMD_MAX_RESPONSE_LEN 4

typedef enum {
    PS2_CMD_RESPONSECODE_ACK = 0xfa,
    PS2_CMD_RESPONSECODE_RESEND = 0xfe,
    PS2_CMD_RESPONSECODE_ERROR = 0xfc
} PS2_CMD_RESPONSECODE;


/* callbacks from underlying protocol layer */
void ps2cmd_busEventHandler(PS2_BUS_EVENT event);
void ps2cmd_busByteReceiveHandler(uint8_t byte);

/* private functions */

/* send another frame if we're in the right state */
void ps2cmd_tryToRebuffer();

/* check if we should continue with the current command or fail. Returns true if we can continue. If false, the command is already cancelled. */
bool ps2cmd_checkContinue();

/* finishes a command successfully */
void ps2cmd_success();

/* Callbacks to upper protocol layer */
ps2cmd_CommandCompletionHandler ps2cmd_completionCallback;
ps2cmd_DataReceivedHandler ps2cmd_dataReceivedCallback;
ps2cmd_IdleHandler ps2cmd_idleCallback;

/* current command state */

typedef enum {
    PS2_CMD_STATE_IDLE,
    PS2_CMD_STATE_READY_TO_SEND,
    PS2_CMD_STATE_WAIT_FOR_SEND_ACK,
    PS2_CMD_STATE_WAIT_FOR_DEVICE_ACK,
    PS2_CMD_STATE_WAIT_FOR_DEVICE_RESPONSE
} PS2_CMD_STATE;

PS2_CMD_STATE ps2cmd_currentState;

uint8_t ps2cmd_currentCommand[PS2_CMD_MAX_COMMAND_LEN];
uint8_t ps2cmd_currentCommandLen = 0;
uint8_t ps2cmd_currentCommandOffset = 0;

uint8_t ps2cmd_currentResponse[PS2_CMD_MAX_RESPONSE_LEN];
uint8_t ps2cmd_currentResponseLen = 0;
uint8_t ps2cmd_currentResponseOffset = 0;

uint8_t ps2cmd_errorCount = 0;
uint8_t ps2cmd_idleCount = 0;

void ps2cmd_init(ps2cmd_DataReceivedHandler dataReceivedHandler, ps2cmd_IdleHandler idleHandler) {
    ps2cmd_dataReceivedCallback = dataReceivedHandler;
    ps2cmd_idleCallback = idleHandler;
    ps2cmd_completionCallback = NULL;
    ps2cmd_currentCommandLen = 0;
    ps2cmd_currentCommandOffset = 0;
    ps2cmd_currentResponseLen = 0;
    ps2cmd_currentResponseOffset = 0;
    ps2cmd_errorCount = 0;
    ps2cmd_idleCount = 0;
    ps2bus_init(ps2cmd_busEventHandler, ps2cmd_busByteReceiveHandler);
}

bool ps2cmd_sendCommand(uint8_t command, uint8_t argLength, uint8_t* args, uint8_t expectedResponseLen, ps2cmd_CommandCompletionHandler completion) {
    if (ps2cmd_commandRunning()) return false;
    if (argLength+1 > PS2_CMD_MAX_COMMAND_LEN) return false;
    if (expectedResponseLen > PS2_CMD_MAX_RESPONSE_LEN) return false;
    ps2cmd_currentCommand[0] = command;
    int i;
    for (i=0; i<argLength; i++) {
        ps2cmd_currentCommand[i+1] = args[i];
    }
    ps2cmd_currentCommandOffset = 0;
    ps2cmd_currentResponseOffset = 0;
    ps2cmd_errorCount = 0;
    ps2cmd_idleCount = 0;
    ps2cmd_currentCommandLen = argLength+1;
    ps2cmd_currentResponseLen = expectedResponseLen;
    ps2cmd_completionCallback = completion;
    ps2cmd_currentState = PS2_CMD_STATE_READY_TO_SEND;
    ps2cmd_tryToRebuffer();
    return true;
}

bool ps2cmd_commandRunning() {
    return (ps2cmd_currentState != PS2_CMD_STATE_IDLE);
}

void ps2cmd_busEventHandler(PS2_BUS_EVENT event) {
    switch (event) {
        case PS2_BUS_GOING_IDLE:
            ps2cmd_tryToRebuffer();
            break;
        case PS2_BUS_IDLING:
            if (ps2cmd_currentState == PS2_CMD_STATE_IDLE) {
                if (ps2cmd_idleCallback) ps2cmd_idleCallback();
            } else {
                ps2cmd_idleCount++;
                ps2cmd_checkContinue();
                ps2cmd_tryToRebuffer();
            }
            break;
        case PS2_BUS_FRAME_ACKED:
            if (ps2cmd_currentState == PS2_CMD_STATE_WAIT_FOR_SEND_ACK) {
                ps2cmd_currentState = PS2_CMD_STATE_WAIT_FOR_DEVICE_ACK;
            }
            break;
        case PS2_BUS_START_ERROR:
        case PS2_BUS_PARITY_ERROR:
        case PS2_BUS_STOP_ERROR:    //Incoming frame failed - it's already cancelled and we wait for resend
            ps2cmd_errorCount++;
            ps2cmd_checkContinue();
            break;
        case PS2_BUS_TIMEOUT_ERROR:
            ps2cmd_idleCount = PS2_CMD_MAX_IDLE_COUNT + 1;  //Force timeout error
            ps2cmd_checkContinue();                         //Will always bail
            break;
        case PS2_BUS_NACK_ERROR:    //Outgoing frame failed - we should try to resend the last byte
            ps2cmd_errorCount++;
            if (ps2cmd_checkContinue()) {
                ps2cmd_currentState = PS2_CMD_STATE_READY_TO_SEND;
            }
            break;
    }
}

void ps2cmd_busByteReceiveHandler(uint8_t byte) {
    if (ps2cmd_currentState == PS2_CMD_STATE_WAIT_FOR_DEVICE_ACK) {
        if (byte == PS2_CMD_RESPONSECODE_ACK) {
            ps2cmd_currentCommandOffset++;
            if (ps2cmd_currentCommandOffset < ps2cmd_currentCommandLen) ps2cmd_currentState = PS2_CMD_STATE_READY_TO_SEND;
            else if (ps2cmd_currentResponseLen > 0) ps2cmd_currentState = PS2_CMD_STATE_WAIT_FOR_DEVICE_RESPONSE;
            else ps2cmd_success();
        } else if (byte == PS2_CMD_RESPONSECODE_RESEND) {
            ps2cmd_errorCount++;
            if (ps2cmd_checkContinue()) {
                ps2cmd_currentState = PS2_CMD_STATE_READY_TO_SEND;
            }
        } else { //PS2_CMD_RESPONSECODE_ERROR or anything else
            ps2cmd_errorCount = PS2_CMD_MAX_ERROR_COUNT + 1;    //force error failure
            ps2cmd_checkContinue();                             //Will always bail
        }
    } else if (ps2cmd_currentState == PS2_CMD_STATE_WAIT_FOR_DEVICE_RESPONSE) {
        ps2cmd_currentResponse[ps2cmd_currentResponseOffset] = byte;
        ps2cmd_currentResponseOffset++;
        if (ps2cmd_currentResponseOffset >= ps2cmd_currentResponseLen) ps2cmd_success();
    } else if (ps2cmd_currentState == PS2_CMD_STATE_IDLE) {
        if (ps2cmd_dataReceivedCallback) ps2cmd_dataReceivedCallback(byte);
    }
}

void ps2cmd_tryToRebuffer() {
    if (ps2cmd_currentState == PS2_CMD_STATE_READY_TO_SEND) {
        if (ps2cmd_currentCommandOffset < ps2cmd_currentCommandLen) {
            if (ps2bus_sendByte(ps2cmd_currentCommand[ps2cmd_currentCommandOffset])) {
                ps2cmd_currentState = PS2_CMD_STATE_WAIT_FOR_SEND_ACK;
            }
            //Note: We don't increase ps2cmd_currentCommandOffset, it's done after the send was acked and the device sends an ACK response
        }
    }
}

bool ps2cmd_checkContinue() {
    if (ps2cmd_currentState == PS2_CMD_STATE_IDLE) return; //Nothing to cancel
    if (ps2cmd_errorCount >= PS2_CMD_MAX_ERROR_COUNT) {
        ps2cmd_CommandCompletionHandler cb = ps2cmd_completionCallback;
        ps2cmd_completionCallback = NULL;
        ps2cmd_currentState = PS2_CMD_STATE_IDLE;
        if (cb) cb(PS2_CMD_COMPLETION_FAIL, ps2cmd_currentResponseOffset, ps2cmd_currentResponse);
        return false;
    } else if (ps2cmd_idleCount >= PS2_CMD_MAX_IDLE_COUNT) {
        ps2cmd_CommandCompletionHandler cb = ps2cmd_completionCallback;
        ps2cmd_completionCallback = NULL;
        ps2cmd_currentState = PS2_CMD_STATE_IDLE;
        if (cb) cb(PS2_CMD_COMPLETION_TIMEOUT, ps2cmd_currentResponseOffset, ps2cmd_currentResponse);
        return false;
    } else return true;
}

void ps2cmd_success() {
    ps2cmd_CommandCompletionHandler cb = ps2cmd_completionCallback;
    ps2cmd_completionCallback = NULL;
    ps2cmd_currentState = PS2_CMD_STATE_IDLE;
    if (cb) cb(PS2_CMD_COMPLETION_OK, ps2cmd_currentResponseOffset, ps2cmd_currentResponse);
}
