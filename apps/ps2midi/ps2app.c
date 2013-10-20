#include "ps2app.h"
#include "ps2scancodes.h"

#define PS2_APP_WAIT_UNKNOWN_TIME 10 /* idle counts to wait for mouse id - if exceeded, assume keyboard */
#define PS2_APP_MAX_EVENT_LENGTH 10
#define PS2_APP_EVENT_IDLE_TIMEOUT 2 /* if we encounter this number of IDLE messages, we clear the event buffer */
#define PS2_APP_IDLE_PING 20 /* if we haven't heard anything from the device, see if it's still there */

typedef enum {
    PS2_CMDCODE_RESET = 0xff,
    PS2_CMDCODE_SET_DEFAULT = 0xf6,
    PS2_CMDCODE_DISABLE_REPORTING = 0xf5,   //No data, both mouse and keyboard
    PS2_CMDCODE_ENABLE_REPORTING = 0xf4,    //No data, both mouse and keyboard
    PS2_CMDCODE_KEYBOARD_SETLEDS = 0xed,    //1 byte data: 1=scroll lock, 2=num lock, 4=caps lock
    PS2_CMDCODE_KEYBOARD_SETDELAYREPEAT = 0xf3, //1 byte data: Bit 0..4:Repeat(0=fastest), 5..6:Delay(0=fastest)
    PS2_CMDCODE_MOUSE_SETSAMPLERATE = 0xf3, //1 byte data: Samples/s (valid: 10, 20, 40, 60, 80, 100, 200)
    PS2_CMDCODE_MOUSE_SETRESOLUTION = 0xe8, //1 byte data: 0=1/mm, 1=2/mm 2=4/mm, 3=8/mm
    PS2_CMDCODE_MOUSE_SETSCALE21 = 0xe7,    //No data, sets scaling to 2:1
    PS2_CMDCODE_MOUSE_SETSCALE11 = 0xe6,    //No data, sets scaling to 1:1
    PS2_CMDCODE_MOUSE_GETDEVICEID = 0xf2    //No data, mouse responds with 0 (or 3 after magic intellimouse setup)
} PS2_COMMANDCODE;

/* handlers from lower layer */

void ps2app_cmdCompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);
void ps2app_cmdDataHandler(uint8_t data);
void ps2app_cmdIdleHandler();

/* private functions */

/* goto a new state (if not yet in that state), notify client and trigger further action */
void ps2app_gotoConnState(PS2_APP_CONNECTION newConnState);

/* check whether the collected event data qualifies as an event */
void ps2app_checkMouseEvent();
void ps2app_checkKeyboardEvent();


/* client callbacks */

ps2app_ConnectionChangeHandler ps2app_connectionCallback;
ps2app_MouseInputHandler ps2app_mouseInputCallback;
ps2app_KeyboardInputHandler ps2app_keyboardInputCallback;

/* current state */

PS2_APP_CONNECTION ps2app_connection = PS2_APP_CONN_NONE;

uint32_t ps2app_idleCounter = 0;
uint8_t ps2app_initSeq = 0;
bool ps2app_mouse_haveScroll = false;

uint8_t ps2app_eventData[PS2_APP_MAX_EVENT_LENGTH];
uint8_t ps2app_eventDataLen = 0;

void ps2app_init(ps2app_ConnectionChangeHandler connCB, ps2app_MouseInputHandler mouseCB, ps2app_KeyboardInputHandler kbCB) {
    ps2app_connectionCallback = connCB;
    ps2app_mouseInputCallback = mouseCB;
    ps2app_keyboardInputCallback = kbCB;
    ps2app_idleCounter = 0;
    ps2app_initSeq = 0;
    ps2app_connection = PS2_APP_CONN_INVALID;   //Force gotoConnState to trigger
    ps2app_gotoConnState(PS2_APP_CONN_NONE);
    ps2cmd_init(&ps2app_cmdDataHandler, ps2app_cmdIdleHandler);
}

void ps2app_cmdDataHandler(uint8_t data) {
    ps2app_idleCounter = 0;
    if ((ps2app_connection == PS2_APP_CONN_NONE) && (data == 0xaa)) {
        ps2app_gotoConnState(PS2_APP_CONN_UNKNOWN);
    } else if ((ps2app_connection == PS2_APP_CONN_UNKNOWN) && (data == 0x00)) {
        ps2app_gotoConnState(PS2_APP_CONN_INIT_MOUSE);
    } else if ((ps2app_connection == PS2_APP_CONN_RUN_MOUSE) || (ps2app_connection == PS2_APP_CONN_RUN_KEYBOARD)) {    //streaming event data
        if (ps2app_eventDataLen >= PS2_APP_MAX_EVENT_LENGTH) ps2app_eventDataLen = 0;
        ps2app_eventData[ps2app_eventDataLen] = data;
        ps2app_eventDataLen++;
        ps2app_checkMouseEvent();
        ps2app_checkKeyboardEvent();
    }
}

void ps2app_cmdIdleHandler() {
    ps2app_idleCounter++;
    if ((ps2app_connection == PS2_APP_CONN_UNKNOWN) && (ps2app_idleCounter > PS2_APP_WAIT_UNKNOWN_TIME)) {
        ps2app_gotoConnState(PS2_APP_CONN_INIT_KEYBOARD);
    }
    if ((ps2app_connection == PS2_APP_CONN_RUN_KEYBOARD) || (ps2app_connection == PS2_APP_CONN_RUN_MOUSE)) {
        if (ps2app_idleCounter >= PS2_APP_EVENT_IDLE_TIMEOUT) { //event timed out
            ps2app_eventDataLen = 0;
        }
        if (ps2app_idleCounter >= PS2_APP_IDLE_PING) {          //Should send something to the device to see if it's still there
            //TODO **************
        }
    }
}

void ps2app_mouse1CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
    //TODO: Do further setup ***************
    switch (completionCode) {
        case PS2_CMD_COMPLETION_FAIL: ps2app_gotoConnState(PS2_APP_CONN_INVALID); break;
        case PS2_CMD_COMPLETION_TIMEOUT : ps2app_gotoConnState(PS2_APP_CONN_NONE); break;
        case PS2_CMD_COMPLETION_OK: ps2app_gotoConnState(PS2_APP_CONN_RUN_MOUSE); break;
    }
}

void ps2app_keyboard1CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
    switch (completionCode) {
        case PS2_CMD_COMPLETION_FAIL: ps2app_gotoConnState(PS2_APP_CONN_INVALID); break;
        case PS2_CMD_COMPLETION_TIMEOUT : ps2app_gotoConnState(PS2_APP_CONN_NONE); break;
        case PS2_CMD_COMPLETION_OK: ps2app_gotoConnState(PS2_APP_CONN_RUN_KEYBOARD); break;
    }
}

void ps2app_gotoConnState(PS2_APP_CONNECTION newConnState) {
    if (ps2app_connection == newConnState) return;
    ps2app_connection = newConnState;
    ps2app_idleCounter = 0;
    if (ps2app_connectionCallback) ps2app_connectionCallback(ps2app_connection);
    if (ps2app_connection == PS2_APP_CONN_INIT_KEYBOARD) ps2cmd_sendCommand(PS2_CMDCODE_ENABLE_REPORTING, 0, NULL, 0, ps2app_keyboard1CompletionHandler);
    if (ps2app_connection == PS2_APP_CONN_INIT_MOUSE) ps2cmd_sendCommand(PS2_CMDCODE_ENABLE_REPORTING, 0, NULL, 0, ps2app_mouse1CompletionHandler);
}


void ps2app_checkMouseEvent() {
    if (ps2app_connection != PS2_APP_CONN_RUN_MOUSE) return;
    uint8_t eventLen = ps2app_mouse_haveScroll ? 4 : 3;
    if (ps2app_eventDataLen >= eventLen) {
        int16_t dx = ps2app_eventData[1];
        if (ps2app_eventData[0] & 0x10) dx -= 256;
        int16_t dy = ps2app_eventData[2];
        if (ps2app_eventData[0] & 0x20) dy -= 256;
        int16_t dz = (ps2app_mouse_haveScroll) ? ((int8_t*)ps2app_eventData)[3] : 0;
        bool left = (ps2app_eventData[0] & 0x01) ? true : false;
        bool right = (ps2app_eventData[0] & 0x02) ? true : false;
        bool middle = (ps2app_eventData[0] & 0x04) ? true : false;
        if (ps2app_mouseInputCallback) ps2app_mouseInputCallback(dx,dy,dz,left,right,middle);
        ps2app_eventDataLen = 0;
    }
}

void ps2app_checkKeyboardEvent() {
    if (ps2app_connection != PS2_APP_CONN_RUN_KEYBOARD) return;
    uint16_t idx = 0;
    while (ps2ScanCodesMode2[idx] != 0) {
        uint8_t codeLen = 0x0f & ps2ScanCodesMode2[idx];
        if (codeLen != ps2app_eventDataLen) continue;
        uint8_t i;  //length match - check code
        for (i=0;i<codeLen;i++) {
            if (ps2ScanCodesMode2[idx+1+i] != ps2app_eventData[i]) break;
        }
        if (i == codeLen) { //code match
            bool down = (ps2ScanCodesMode2[idx] & 0x10) ? false : true;
            uint8_t keyCode = ps2ScanCodesMode2[idx+1+codeLen];
            if (ps2app_keyboardInputCallback) ps2app_keyboardInputCallback(keyCode, down);
            ps2app_eventDataLen = 0;
            break;  //We're done searching
        }
    }
}

