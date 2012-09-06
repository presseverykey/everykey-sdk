/** Synchronous Serial Port */

#ifndef _SSP_
#define _SSP_

#include "types.h"
#include "memorymap.h"

/** inits the SSP0 unit.
 * @param clockDiv SSP clock divider (1..255). SSP bus clock = main clock / (2 * clockDiv)
 * @param datasize data size per transfer unit (4..16 bits, usually 8)
 * @param frameformat frame format to use (TI, SPI or microwire)
 * @param idleClockHigh if true, idle clock level is high, low otherwise
 * @param dataOnSecond if true, data is at at the second clock transition (rising edge if idle is high), first otherwise
 * @param clockScaler clock scale (1..256) SSP clock is divided by this value again to build the actual bit clock 
 * @param master if true, this is the bus master, slave if false
 */
void SSP_Init(uint8_t clockDiv, uint8_t datasize, SSP_CR0_VALUES frameformat, bool idleClockHigh, bool dataOnSecond, bool master);

/** writes a frame to the SSP port 
 * @param value frame to write (4..16 bits) */
void SSP_Write(uint16_t value);

/** reads a frame from the SPI port 
 * @return read frame (4..16 bits) */
uint16_t SSP_Read();


#endif
