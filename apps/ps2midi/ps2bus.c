#include "ps2bus.h"

#define PS2_PORT 0
#define PS2_CLOCK_PIN 4
#define PS2_DATA_PIN 5

#define PS2_BUS_IDLE_TIME_US 50000 /* Timeout to send "idling" event (no activity on bus) - 50ms */
#define PS2_BUS_MAX_READ_FRAME_TIME_US 2000 /* From start to end, a read frame should finish within 2ms */
#define PS2_BUS_MAX_WRITE_FRAME_TIME_US 20000 /* outgoing frames should complete within 20ms (from start of RTS) */
#define PS2_BUS_RTS_TIME_US 100 /* Pull down clock low for 100us to indicate RTS or abort transaction */


typedef enum {
	PS2_BUS_IDLE,
	PS2_BUS_READING,	//buffer valid, frame counter valid
	PS2_BUS_WRITING,	//buffer valid, frame counter valid
	PS2_BUS_RTS,		//buffer valid, frame counter valid
	PS2_BUS_ABORTING
} PS2_BUS_STATE;

/* current bus state */
PS2_BUS_STATE ps2bus_state = PS2_BUS_IDLE;

/* When reading, a read buffer. When writing, a write buffer */
uint8_t ps2bus_frameBuffer;	

/*	bits elapsed in current frame transfer
	0: start bit, 1..8: data bits, 9:parity, 10:stop, (11:ack) */
uint8_t ps2bus_frameBitCounter;

/* parity counter for read and write */
bool ps2bus_odd;

/* callback pointer set by client */
ps2bus_EventCallback ps2bus_eventCallback;

/* callback pointer set by client */
ps2bus_ByteReceivedCallback ps2bus_byteReceivedCallback;

// starts the timer, us units
void ps2bus_startTimer(uint32_t us);

// stops the timer
void ps2bus_stopTimer();

// aborts the current transaction
void ps2bus_abortTransaction();

// call to go back to idle state
void ps2bus_goIdle();

//callback forwarder when an event or state change occurret
void ps2bus_forwardEvent(PS2_BUS_EVENT state);

//callback forwarder when a byte transmission was completed
void ps2bus_forwardByteReceived(uint8_t value);

void ps2bus_init(ps2bus_EventCallback eventCB, ps2bus_ByteReceivedCallback receiveCB) {
//	TOGGLE_LED

    ps2bus_eventCallback = eventCB;
    ps2bus_byteReceivedCallback = receiveCB;
	ANY_GPIO_SET_FUNCTION(PS2_PORT, PS2_CLOCK_PIN, PIO, IOCON_IO_ADMODE_DIGITAL);
	ANY_GPIO_SET_FUNCTION(PS2_PORT, PS2_DATA_PIN, PIO, IOCON_IO_ADMODE_DIGITAL); 
	any_gpio_set_dir(PS2_PORT, PS2_CLOCK_PIN, INPUT);
	any_gpio_set_dir(PS2_PORT, PS2_DATA_PIN, INPUT);
	any_gpio_set_interrupt_mode(PS2_PORT, PS2_CLOCK_PIN, TRIGGER_FALLING_EDGE);
	Timer_Enable(CT16B0, true);
	Timer_SetPrescale(CT16B0, 72);	//us units
	Timer_SetMatchBehaviour(CT16B0, 0, TIMER_MATCH_INTERRUPT | TIMER_MATCH_STOP);
	ps2bus_state = PS2_BUS_IDLE;
    NVIC_EnableInterrupt(NVIC_PIO_0);
    NVIC_EnableInterrupt(NVIC_CT16B0);
}

bool ps2bus_sendByte(uint8_t byte) {
	//TODO: Should we start writing if a read is in progress? Maybe yes...
	if (ps2bus_state != PS2_BUS_IDLE) return false;
	ps2bus_state = PS2_BUS_RTS;
	any_gpio_set_dir(PS2_PORT, PS2_DATA_PIN, OUTPUT);
	any_gpio_write(PS2_PORT, PS2_DATA_PIN, 0);
	any_gpio_set_dir(PS2_PORT, PS2_CLOCK_PIN, OUTPUT);
	any_gpio_write(PS2_PORT, PS2_CLOCK_PIN, 0);
	ps2bus_frameBuffer = byte;
	ps2bus_frameBitCounter = 0;
	ps2bus_odd = 0;
	ps2bus_startTimer(PS2_BUS_RTS_TIME_US);
	return true;
}


void gpio0_handler(void) {

	uint32_t mask = any_gpio_get_interrupt_mask(0);
	any_gpio_clear_interrupt_mask(0, mask);
	if (!(mask & (1<<PS2_CLOCK_PIN))) return;	//not our pin

	bool bit = any_gpio_read(PS2_PORT, PS2_DATA_PIN);
	switch (ps2bus_state) {
		case PS2_BUS_IDLE:	
			if (bit) {
				ps2bus_forwardEvent(PS2_BUS_START_ERROR);
				ps2bus_abortTransaction();
			} else {
				ps2bus_state = PS2_BUS_READING;
				ps2bus_frameBuffer = 0;
				ps2bus_odd = false;
				ps2bus_frameBitCounter = 0;
				ps2bus_startTimer(PS2_BUS_MAX_READ_FRAME_TIME_US);	//Should be finished in 2ms
			}
			break;
		case PS2_BUS_READING:
			ps2bus_frameBitCounter++;
			if (ps2bus_frameBitCounter <= 8) {			//data bits
				if (bit) {
					ps2bus_frameBuffer |= 1<<(ps2bus_frameBitCounter-1);
					ps2bus_odd = !ps2bus_odd;
				}
			} else if (ps2bus_frameBitCounter == 9) {	//parity bit
				if (bit == ps2bus_odd) {
					ps2bus_forwardEvent(PS2_BUS_PARITY_ERROR);
					ps2bus_abortTransaction();
				}
			} else if (ps2bus_frameBitCounter == 10) {	//stop bit
				if (!bit) {
					ps2bus_forwardEvent(PS2_BUS_STOP_ERROR);
					ps2bus_abortTransaction();
				}
				else {
					ps2bus_forwardByteReceived(ps2bus_frameBuffer);
					ps2bus_goIdle();
				}
			}
			break;
		case PS2_BUS_WRITING:
			ps2bus_frameBitCounter++;   //start bit was already sent with the RTS
			if (ps2bus_frameBitCounter <= 8) {			//data bits
				bit = ((ps2bus_frameBuffer & (1 << (ps2bus_frameBitCounter-1))) != 0);
				any_gpio_write(PS2_PORT, PS2_DATA_PIN, bit);
				if (bit) ps2bus_odd = !ps2bus_odd;
			} else if (ps2bus_frameBitCounter == 9) {	//parity bit
				any_gpio_write(PS2_PORT, PS2_DATA_PIN, !ps2bus_odd);
			} else if (ps2bus_frameBitCounter == 10) {	//stop bit - switch to input
				any_gpio_write(PS2_PORT, PS2_DATA_PIN, true);   //Strictly, this line is not needed - just for debugging
				any_gpio_set_dir(PS2_PORT, PS2_DATA_PIN, INPUT);
			} else if (ps2bus_frameBitCounter == 11) {	//ack bit - read
				if (bit) ps2bus_forwardEvent(PS2_BUS_NACK_ERROR);
				else ps2bus_forwardEvent(PS2_BUS_FRAME_ACKED);
				ps2bus_goIdle();
			}
			break;
		case PS2_BUS_RTS:
		case PS2_BUS_ABORTING:    //in RTS and ABORTING states, we're pulling clock low, so it's by us - no need to react
			break;
	}
}

/* timer fired - either a wakeup (we've pulled down the clock long enough)
or a transaction timeout (device didn't clock fast enough) */
void ct16b0_handler(void) {
//	TOGGLE_LED

    uint32_t mask = Timer_GetInterruptMask(CT16B0);
	Timer_ClearInterruptMask(CT16B0, mask);
	//TODO: check mask ***************
	switch (ps2bus_state) {
		case PS2_BUS_RTS:		//We're done pulling the clock low to indicate RTS. We can send now.
			ps2bus_state = PS2_BUS_WRITING;
			any_gpio_write(PS2_PORT, PS2_CLOCK_PIN, true);         //Strictly, this line is not needed - just for debugging
			any_gpio_set_dir(PS2_PORT, PS2_CLOCK_PIN, INPUT);
			ps2bus_startTimer(PS2_BUS_MAX_WRITE_FRAME_TIME_US);   //set timeout to complete next frame
			break;
		case PS2_BUS_ABORTING:
			ps2bus_goIdle();	//We're done pulling the clock lowgoIdle will cleanup the rest 
			break;
		case PS2_BUS_READING:
		case PS2_BUS_WRITING:
			ps2bus_forwardEvent(PS2_BUS_TIMEOUT_ERROR);
			ps2bus_abortTransaction();
			break;
        case PS2_BUS_IDLE:
            ps2bus_forwardEvent(PS2_BUS_IDLING);
            ps2bus_startTimer(PS2_BUS_IDLE_TIME_US);
            break;

	}
}

void ps2bus_startTimer(uint32_t us) {
	Timer_Stop(CT16B0);
	Timer_Reset(CT16B0);
	Timer_SetMatchValue(CT16B0, 0, us);
	Timer_Start(CT16B0);
}

void ps2bus_stopTimer() {
	Timer_Stop(CT16B0);
}

void ps2bus_abortTransaction() {
	any_gpio_set_dir(PS2_PORT, PS2_DATA_PIN, INPUT);	//Shouldn't happen, but make sure we don't accidentally pull data low
	any_gpio_set_dir(PS2_PORT, PS2_CLOCK_PIN, OUTPUT);
	any_gpio_write(PS2_PORT, PS2_CLOCK_PIN, 0);
	ps2bus_state = PS2_BUS_ABORTING;
	ps2bus_startTimer(PS2_BUS_RTS_TIME_US);
}

void ps2bus_goIdle() {
	if (ps2bus_state != PS2_BUS_IDLE) {
		ps2bus_stopTimer();
        any_gpio_write(PS2_PORT, PS2_DATA_PIN, true);   //Strictly, this line is not needed - just for debugging
        any_gpio_write(PS2_PORT, PS2_CLOCK_PIN, true);  //Strictly, this line is not needed - just for debugging
		any_gpio_set_dir(PS2_PORT, PS2_DATA_PIN, INPUT);
		any_gpio_set_dir(PS2_PORT, PS2_CLOCK_PIN, INPUT);
		ps2bus_state = PS2_BUS_IDLE;
		ps2bus_startTimer(PS2_BUS_IDLE_TIME_US);        //Start idle timer
		ps2bus_forwardEvent(PS2_BUS_GOING_IDLE);        //Send out notification last so that callbacks can initiate new activities
	}
}

void ps2bus_forwardEvent(PS2_BUS_EVENT event) {
    if (ps2bus_eventCallback) ps2bus_eventCallback(event);
}

void ps2bus_forwardByteReceived(uint8_t value) {
    if (ps2bus_byteReceivedCallback) ps2bus_byteReceivedCallback(value);
}
