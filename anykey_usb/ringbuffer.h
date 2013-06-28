#ifndef _RINGBUFFER_
#define _RINGBUFFER_

/** a simple ring buffer implementation */
 

#include "../anykey/types.h"

/** runtime-dynamic components of the ring buffer - must be in RAM */
typedef struct RingBufferDynamic {
	uint16_t readIdx;       //index of next data to be read
	uint16_t writeIdx;      //index of next data to be written
	//If readIdx and writeIdx are equal, the buffer is considered empty.
	uint8_t data[1];        //resize memory for this structure to fit
} RingBufferDynamic;

/** static components of the ring buffer - may be in Flash */
typedef struct RingBufferStatic {
	uint16_t length;			//size in bytes
	RingBufferDynamic* dynamic;	//pointer to dynamic components. Must be large enough for structure with data length
} RingBufferStatic;

/** clears and resets the ring buffer */
void RingBufferInit(const RingBufferStatic* rb);

/** returns the number of bytes available for reading */
uint16_t RingBufferReadBytesAvailable(const RingBufferStatic* rb);

/** returns the number of bytes available for writing */
uint16_t RingBufferWriteBytesAvailable(const RingBufferStatic* rb);

/** reads a single byte into a ring buffer.
 @param rb ring buffer
 @param data data to return by reference
 @return true if the data could be read, false otherwise */
bool RingBufferReadByte(const RingBufferStatic* rb, uint8_t* data);

/** writes a single byte into a ring buffer.
 @param rb ring buffer
 @param data data to insert
 @return true if the data could be written, false otherwise */
bool RingBufferWriteByte(const RingBufferStatic* rb, uint8_t data);

/** reads bytes from a ring buffer. Reads partially if the data does not fit.
 @param rb ring buffer
 @param data pointer to data to write to
 @param length (max) number of bytes to read
 @return the number of bytes actually read from the ring buffer */
uint16_t RingBufferReadBuffer(const RingBufferStatic* rb, uint8_t* data, uint16_t length);

/** writes bytes into a ring buffer. Writes partially if the data does not fit.
 @param rb ring buffer
 @param data pointer to data to read from
 @param length (max) number of bytes to write
 @return the number of bytes actually written into the ring buffer */
uint16_t RingBufferWriteBuffer(const RingBufferStatic* rb, const uint8_t* data, uint16_t length);

#endif	
