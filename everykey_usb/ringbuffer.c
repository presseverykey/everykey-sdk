#include "ringbuffer.h"

void RingBufferInit(const RingBufferStatic* rb) {
	rb->dynamic->readIdx = 0;
	rb->dynamic->writeIdx = 0;
}

/** returns the number of bytes available for reading */
uint16_t RingBufferReadBytesAvailable(const RingBufferStatic* rb) {
	RingBufferDynamic* rbdyn = rb->dynamic;
	uint16_t rIdx = rbdyn->readIdx;
	uint16_t wIdx = rbdyn->writeIdx;
	return (((rb->length)+wIdx)-rIdx) % (rb->length);
}

/** returns the number of bytes available for writing */
uint16_t RingBufferWriteBytesAvailable(const RingBufferStatic* rb) {
	return (rb->length)- RingBufferReadBytesAvailable(rb) - 1;	//-1 to be safe
}

/** reads a single byte into a ring buffer.
 @param rb ring buffer
 @param data data to return by reference
 @return true if the data could be read, false otherwise */
bool RingBufferReadByte(const RingBufferStatic* rb, uint8_t* data) {
	if (RingBufferReadBytesAvailable(rb) > 0) {
		RingBufferDynamic* rbdyn = rb->dynamic;
		uint16_t idx = rbdyn->readIdx;
		*data = rbdyn->data[idx];
		idx = (idx+1) % (rb->length);
		rbdyn->readIdx = idx;
		return true;
	} else return false;
}

/** writes a single byte into a ring buffer.
 @param rb ring buffer
 @param data data to insert
 @return true if the data could be written, false otherwise */
bool RingBufferWriteByte(const RingBufferStatic* rb, uint8_t data) {
	if (RingBufferWriteBytesAvailable(rb) > 0) {
		RingBufferDynamic* rbdyn = rb->dynamic;
		uint16_t idx = rbdyn->writeIdx;
		rbdyn->data[idx] = data;
		idx = (idx+1) % (rb->length);
		rbdyn->writeIdx = idx;
		return true;
	} else return false;
}

/** reads bytes from a ring buffer. Reads partially if the data does not fit.
 @param rb ring buffer
 @param data pointer to data to write to
 @param length (max) number of bytes to read
 @return the number of bytes actually read from the ring buffer */
uint16_t RingBufferReadBuffer(const RingBufferStatic* rb, uint8_t* data, uint16_t length) {
	int read = 0;
	while (read < length) {
		if (!RingBufferReadByte(rb, &(data[read]))) break;
		else read++;
	}
	return read;
}

/** writes bytes into a ring buffer. Writes partially if the data does not fit.
 @param rb ring buffer
 @param data pointer to data to read from
 @param length (max) number of bytes to write
 @return the number of bytes actually written into the ring buffer */
uint16_t RingBufferWriteBuffer(const RingBufferStatic* rb, const uint8_t* data, uint16_t length) {
	int written = 0;
	while (written < length) {
		if (!RingBufferWriteByte(rb, data[written])) break;
		else written++;
	}
	return written;
}


