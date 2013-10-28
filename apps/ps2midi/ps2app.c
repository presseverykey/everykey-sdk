#include "ps2app.h"
#include "ps2scancodes.h"

#define PS2_APP_WAIT_UNKNOWN_TIME 10 /* idle counts to wait for mouse id - if exceeded, assume keyboard */
#define PS2_APP_MAX_EVENT_LENGTH 10
#define PS2_APP_EVENT_IDLE_TIMEOUT 2 /* if we encounter this number of IDLE messages, we clear the event buffer */
#define PS2_APP_IDLE_PING 20 /* if we haven't heard anything from the device, see if it's still there */
#define PS2_APP_RESET_TIMEOUT 20 /* timeout to send a reset for all states other than running */

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

/* do whatever is appropriate for a given key event */
void ps2app_keyboardInput(uint8_t keyCode, bool down);

/* sets the LEDs to reflect our current state */
void ps2app_updateKeyboardLeds();



/* client callbacks */

ps2app_ConnectionChangeHandler ps2app_connectionCallback;
ps2app_MouseInputHandler ps2app_mouseInputCallback;
ps2app_KeyboardInputHandler ps2app_keyboardInputCallback;
ps2app_IdleHandler ps2app_idleCallback;

/* current state */

PS2_APP_CONNECTION ps2app_connection = PS2_APP_CONN_NONE;

uint32_t ps2app_idleCounter = 0;
uint8_t ps2app_initSeq = 0;
bool ps2app_mouse_haveScroll = false;       //4 byte reports, scroll value in byte 3, bits 0..3
bool ps2app_mouse_haveMoreButtons = false;  //4 byte reports, button 4 and 5 available (byte 3, bit 4/5)
uint8_t ps2app_keyboard_ledState = 0;

uint8_t ps2app_eventData[PS2_APP_MAX_EVENT_LENGTH];
uint8_t ps2app_eventDataLen = 0;

/* various command completion handlers */

void ps2app_pingCompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);
void ps2app_resetCompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);
void ps2app_mouse1CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);
void ps2app_mouse2CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);
void ps2app_mouse3CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);
void ps2app_mouse4CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);
void ps2app_mouse5CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);
void ps2app_keyboard1CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);
void ps2app_keyboardLedsCompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response);


void ps2app_init(   ps2app_ConnectionChangeHandler connCB,
                    ps2app_MouseInputHandler mouseCB,
                    ps2app_KeyboardInputHandler kbCB,
                    ps2app_IdleHandler idleCB) {
    ps2app_connectionCallback = connCB;
    ps2app_mouseInputCallback = mouseCB;
    ps2app_keyboardInputCallback = kbCB;
    ps2app_idleCallback = idleCB;
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
    if (ps2app_idleCallback) ps2app_idleCallback();

    ps2app_idleCounter++;
    switch (ps2app_connection) {
        case PS2_APP_CONN_UNKNOWN:
            if (ps2app_idleCounter > PS2_APP_WAIT_UNKNOWN_TIME) {   //No second byte sent -> assume keyboard
                ps2app_gotoConnState(PS2_APP_CONN_INIT_KEYBOARD);
            }
            break;
        case PS2_APP_CONN_RUN_KEYBOARD:
        case PS2_APP_CONN_RUN_MOUSE:
            if (ps2app_idleCounter >= PS2_APP_EVENT_IDLE_TIMEOUT) { //half-sent event data, clear it
                ps2app_eventDataLen = 0;
            }
            if (ps2app_idleCounter >= PS2_APP_IDLE_PING) {          
                //Device has been idle for some time - talk to it to see if it is still there
                ps2app_idleCounter = 0;
                ps2cmd_sendCommand(PS2_CMDCODE_ENABLE_REPORTING, 0, NULL, 0, ps2app_pingCompletionHandler);
            }
            break;
        default:    //Stuck device or nothing detected on bus
            if (ps2app_idleCounter >= PS2_APP_RESET_TIMEOUT) {
                ps2app_idleCounter = 0;
                ps2cmd_sendCommand(PS2_CMDCODE_RESET, 0, NULL, 0, ps2app_resetCompletionHandler);
            }
            break;
    }
}

void ps2app_gotoConnState(PS2_APP_CONNECTION newConnState) {
    if (ps2app_connection == newConnState) return;
    ps2app_connection = newConnState;
    ps2app_idleCounter = 0;
    ps2app_eventDataLen = 0;    //No data taken from one to another state
    if (ps2app_connectionCallback) ps2app_connectionCallback(ps2app_connection);
    
    switch (ps2app_connection) {
        case PS2_APP_CONN_INIT_KEYBOARD:
            ps2cmd_sendCommand(PS2_CMDCODE_ENABLE_REPORTING, 0, NULL, 0, ps2app_keyboard1CompletionHandler);
            break;
        case PS2_APP_CONN_INIT_MOUSE:
            {   
            /* A mouse was detected. Try to enable the scroll wheel by the magic Intellimouse sequence:
            Set sample rate 200, set sample rate 100, set sample rate 80, read device id. The response
            will tell if the device understood this sequence and enabled the wheel. Then enable reporting. */
                ps2app_mouse_haveScroll = false;    //assume no scroll wheel
                ps2app_mouse_haveMoreButtons = false;    //assume only 3 basic buttons
                uint8_t sampleRate = 200;
                ps2cmd_sendCommand(PS2_CMDCODE_MOUSE_SETSAMPLERATE, 1, &sampleRate, 0, ps2app_mouse1CompletionHandler);
            }
            break;
        case PS2_APP_CONN_RUN_KEYBOARD:
            ps2app_keyboard_ledState = 0;
            ps2app_updateKeyboardLeds();
            break;
    }
}


/* completion handler for mouse or keyboard ping - check if the device is still there */
void ps2app_pingCompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
    ps2app_idleCounter = 0; 
    ps2app_eventDataLen = 0;
    if (completionCode != PS2_CMD_COMPLETION_OK) {  //Ping failed - try to reset
        ps2cmd_sendCommand(PS2_CMDCODE_RESET, 0, NULL, 0, ps2app_resetCompletionHandler);
    }
}

/* completion handler for a reset attempt - either because of malfunction or to check the bus */
void ps2app_resetCompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
    //We'll go to unconnected state no matter if the reset succeeded or not - we will see
    //the normal boot sequence or nothing if no device is attached. Or we'll retry reset later.
    ps2app_gotoConnState(PS2_APP_CONN_NONE);    
}

/* mouse init step 1 complete: sample rate was set to 200, now set to 100 */
void ps2app_mouse1CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
    if (completionCode == PS2_CMD_COMPLETION_OK) {
        uint8_t sampleRate = 100;
        ps2cmd_sendCommand(PS2_CMDCODE_MOUSE_SETSAMPLERATE, 1, &sampleRate, 0, ps2app_mouse2CompletionHandler);
    } else {
        ps2app_gotoConnState(PS2_APP_CONN_NONE);
    }
}

/* mouse init step 2 complete: sample rate was set to 100, now set to 80 */
void ps2app_mouse2CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
    if (completionCode == PS2_CMD_COMPLETION_OK) {
        uint8_t sampleRate = 80;
        ps2cmd_sendCommand(PS2_CMDCODE_MOUSE_SETSAMPLERATE, 1, &sampleRate, 0, ps2app_mouse3CompletionHandler);
    } else {
        ps2app_gotoConnState(PS2_APP_CONN_NONE);
    }
}

/* mouse init step 3 complete: sample rate was set to 80, now read id */
void ps2app_mouse3CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
    if (completionCode == PS2_CMD_COMPLETION_OK) {
        ps2cmd_sendCommand(PS2_CMDCODE_MOUSE_GETDEVICEID, 0, NULL, 1, ps2app_mouse4CompletionHandler);
    } else {
        ps2app_gotoConnState(PS2_APP_CONN_NONE);
    }
}

/* mouse init step 4 complete: device id is returned, set scroll and start reporting */
void ps2app_mouse4CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
    if ((completionCode == PS2_CMD_COMPLETION_OK) && (responseLen == 1)) {
        uint8_t deviceId = response[0];
        ps2app_mouse_haveScroll = (deviceId == 3) || (deviceId == 4);
        ps2app_mouse_haveMoreButtons = (deviceId == 4);
        ps2cmd_sendCommand(PS2_CMDCODE_ENABLE_REPORTING, 0, NULL, 0, ps2app_mouse5CompletionHandler);
    } else {
        ps2app_gotoConnState(PS2_APP_CONN_NONE);
    }
}

/* mouse init step 5 complete: we're done */
void ps2app_mouse5CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
    if (completionCode == PS2_CMD_COMPLETION_OK) {
        ps2app_gotoConnState(PS2_APP_CONN_RUN_MOUSE);
    } else {
        ps2app_gotoConnState(PS2_APP_CONN_NONE);
    }
}

/* keyboard init step 1 complete: we're done */
void ps2app_keyboard1CompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
    if (completionCode == PS2_CMD_COMPLETION_OK) {
        ps2app_gotoConnState(PS2_APP_CONN_RUN_KEYBOARD);
    } else {
        ps2app_gotoConnState(PS2_APP_CONN_NONE);
    }
}



void ps2app_checkMouseEvent() {
    if (ps2app_connection != PS2_APP_CONN_RUN_MOUSE) return;
    uint8_t eventLen = ps2app_mouse_haveScroll ? 4 : 3;
    if (ps2app_eventDataLen >= eventLen) {
        int16_t dx = ps2app_eventData[1];
        if (ps2app_eventData[0] & 0x10) dx -= 256;
        int16_t dy = ps2app_eventData[2];
        if (ps2app_eventData[0] & 0x20) dy -= 256;

        bool left = (ps2app_eventData[0] & 0x01) ? true : false;
        bool right = (ps2app_eventData[0] & 0x02) ? true : false;
        bool middle = (ps2app_eventData[0] & 0x04) ? true : false;

        int16_t dz = 0;
        if (ps2app_mouse_haveScroll) {
            uint8_t fourth = ps2app_eventData[3];
            if (fourth & 0x08) dz = (fourth&0xf) - 16;
            else dz = fourth & 0xf;
            //We might add support for 4th and 5th buttons here.
        }

        if (ps2app_mouseInputCallback) ps2app_mouseInputCallback(dx,dy,dz,left,right,middle);
        ps2app_eventDataLen = 0;
    }
}

void ps2app_checkKeyboardEvent() {
    if (ps2app_connection != PS2_APP_CONN_RUN_KEYBOARD) return;
    uint16_t idx = 0;
    while (true) {
        uint8_t codeLen = 0x0f & ps2ScanCodesMode2[idx];
        if (!codeLen) break;    //Done
        if (codeLen == ps2app_eventDataLen) {
            uint8_t i;  //length match - check code
            for (i=0;i<codeLen;i++) {
                if (ps2ScanCodesMode2[idx+1+i] != ps2app_eventData[i]) break;
            }
            if (i == codeLen) { //code match
                bool down = (ps2ScanCodesMode2[idx] & 0x10) ? false : true;
                uint8_t keyCode = ps2ScanCodesMode2[idx+1+codeLen];
                ps2app_keyboardInput(keyCode, down);
                ps2app_eventDataLen = 0;
                break;  //We're done searching
            }
        }
        idx += codeLen + 2;
    }
}

void ps2app_keyboardInput(uint8_t keyCode, bool down) {
    if (down && (keyCode == KEYCODE_CAPSLOCK)) {
        ps2app_keyboard_ledState ^= PS2_KBD_LED_CAPSLOCK;
        ps2app_updateKeyboardLeds();
    }
    if (down && (keyCode == KEYCODE_KEYPAD_NUMLOCK)) {
        ps2app_keyboard_ledState ^= PS2_KBD_LED_NUMLOCK;
        ps2app_updateKeyboardLeds();
    }
    if (down && (keyCode == KEYCODE_SCROLLLOCK)) {
        ps2app_keyboard_ledState ^= PS2_KBD_LED_SCROLLLOCK;
        ps2app_updateKeyboardLeds();
    }
    if (ps2app_keyboardInputCallback) ps2app_keyboardInputCallback(keyCode, down, ps2app_keyboard_ledState);

}

void ps2app_updateKeyboardLeds() {
    ps2cmd_sendCommand(PS2_CMDCODE_KEYBOARD_SETLEDS, 1, &ps2app_keyboard_ledState, 0, ps2app_keyboardLedsCompletionHandler);
}

void ps2app_keyboardLedsCompletionHandler(PS2_CMD_COMPLETIONCODE completionCode, uint8_t responseLen, uint8_t* response) {
}


