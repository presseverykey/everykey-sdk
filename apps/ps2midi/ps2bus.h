/* PS/2 bus layer driver. This is the lowest protocol layer, bit-banging the PS/2 lines and providing frame transfer */

#ifndef _PS2_BUS_
#define _PS2_BUS_

#include "anykey/anykey.h"

typedef enum {
    PS2_BUS_GOING_IDLE,     //Returning to idle state - this is not an error
    PS2_BUS_IDLING,         //Sent periodically when in idle state (no data) - this is not an error
    PS2_BUS_FRAME_ACKED,    //Byte was successfully sent and acknowledged - this is not an error
	PS2_BUS_START_ERROR = 0x80,    //Start bit not 1 - transaction cancelled
	PS2_BUS_PARITY_ERROR,   //Parity error - transaction cancelled
	PS2_BUS_STOP_ERROR,     //Stop bit not 0 - transaction cancelled
	PS2_BUS_TIMEOUT_ERROR,  //Frame was not completed in time - transaction cancelled
	PS2_BUS_NACK_ERROR,     //Outgoing frame was not acknowledged by device
} PS2_BUS_EVENT;

//called when an event or state change occurret
typedef void (*ps2bus_EventCallback)(PS2_BUS_EVENT state);

//called when a byte transmission was completed
typedef void (*ps2bus_ByteReceivedCallback)(uint8_t value);

//initializes the PS2 bus. Uses the GPIO0 interrupt and CT16B0.
void ps2bus_init(ps2bus_EventCallback eventCB, ps2bus_ByteReceivedCallback receiveCB);

//initiates a send transfer. returns true if the byte was queued successfully.
bool ps2bus_sendByte(uint8_t byte);

#endif

