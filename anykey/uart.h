/* Universal asynchronous receiver transmitter functions - serial port */

#ifndef _UART_
#define _UART_

#include "types.h"

typedef enum {
    PARITY_NONE = 0x00,
    PARITY_ODD = 0x08,
    PARITY_EVEN = 0x18,
    PARITY_ONE = 0x28,
    PARITY_ZERO = 0x38
} UART_PARITY;

/** Initializes the UART peripheral to a specific mode.
@param baud baud rate
@param dataBits number of data bits (5..8)
@param parity parity
@param stopBits (1 or 2 - 2 becomes 1.5 when dataBits = 5)
*/
void UART_Init(uint32_t baud, uint8_t dataBits, UART_PARITY parity, uint8_t stopBits);



#endif
