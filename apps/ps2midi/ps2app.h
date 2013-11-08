/* PS/2 application layer driver. This is the highest layer, on top of ps2cmd. It implements device detection, device initialization and state management.
 It also tries to implement basic hot plugging functions (although PS/2 was not originally intended for hot plugging). */

#ifndef _PS2_APP_
#define _PS2_APP_

#include "ps2cmd.h"


typedef enum {
    PS2_KBD_LED_CAPSLOCK = 0x04,
    PS2_KBD_LED_NUMLOCK = 0x02,
    PS2_KBD_LED_SCROLLLOCK = 0x01
} PS2_KBD_LED_MASK;

typedef enum {
    PS2_APP_CONN_NONE,            //Nothing connected (or not detected yet)
    PS2_APP_CONN_UNKNOWN,         //A device was detected but it's not known yet what it is
    PS2_APP_CONN_INIT_MOUSE,      //A mouse is starting up (or disconnection was not detected yet)
    PS2_APP_CONN_INIT_KEYBOARD,   //A keyboard is starting up (or disconnection was not detected yet)
    PS2_APP_CONN_RUN_MOUSE,       //A mouse is inited and running (or disconnection was not detected yet)
    PS2_APP_CONN_RUN_KEYBOARD,    //A keyboard is inited and running (or disconnection was not detected yet)
    PS2_APP_CONN_INVALID,         //This state never occur except for startup
} PS2_APP_CONNECTION;

typedef void(*ps2app_ConnectionChangeHandler)(PS2_APP_CONNECTION newState);
typedef void(*ps2app_MouseInputHandler)(int16_t dx, int16_t dy, int16_t dz, bool left, bool right, bool middle);
typedef void(*ps2app_KeyboardInputHandler)(uint8_t keycode, bool down, uint8_t leds);
typedef void(*ps2app_IdleHandler)();

void ps2app_init(   ps2app_ConnectionChangeHandler connCB,
                    ps2app_MouseInputHandler mouseCB,
                    ps2app_KeyboardInputHandler kbCB,
                    ps2app_IdleHandler idleCB);




#endif
