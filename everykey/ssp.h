/** Synchronous Serial Port */

#ifndef _SSP_
#define _SSP_

#include "types.h"
#include "memorymap.h"

/** inits the SSP0 unit.
 * @param clockDiv SSP clock divider (1..255). SSP bus clock = main clock / (2 * clockDiv)
 * @param datasize data size per transfer unit (4..16 bits, usually 8)
 * @param frameformat frame format to use (TI, SPI or microwire)
 * @param idleClockHigh if true, idle clock level is high, low otherwise (CPOL)
 * @param dataOnSecond if true, data is at at the second clock transition (rising edge if idle is high), first otherwise (CPHA)
 * @param master if true, this is the bus master, slave if false
 */
void SSP_Init(uint8_t clockDiv, uint8_t datasize, SSP_CR0_VALUES frameformat, bool idleClockHigh, bool dataOnSecond, bool master);

/** writes and reads a frame to the SSP port 
 * @param value frame to write (4..16 bits)
 * @return frame read at the same time */
uint16_t SSP_Transfer(uint16_t value);


#endif
