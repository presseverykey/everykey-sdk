#include "i2c.h"
#include "gpio.h"

#define POINTER_NOT_SET ((void*)-1)

I2C_State* i2c_state = POINTER_NOT_SET;

void I2C_Init(I2C_MODE mode, I2C_State* inState) {
	i2c_state = inState;
	i2c_state->refcon = 0;
	i2c_state->slaveAddress = 0;
	i2c_state->toWrite = 0;
	i2c_state->writeBuffer = NULL;
	i2c_state->toRead = 0;
	i2c_state->readBuffer = NULL;
	i2c_state->completionHandler = NULL;


	uint32_t scl = 360;	//default: 100kbps
	switch (mode) {		//set pin functions and I2C clock rate: 2 * SCL * data rate = CPU clock
		case I2C_MODE_STANDARD:
			scl = 360;
			break;
		case I2C_MODE_FAST:
			scl = 90;
			break;
		case I2C_MODE_FASTPLUS:
			scl = 36;
			break;
	}
	uint32_t func = 0x01 | ((mode == I2C_MODE_FASTPLUS) ? 0x200 : 0);
	IOCON->PIO0_4 = func;
	IOCON->PIO0_5 = func;

	// release reset. Note that there is/was an error in the NXP User Manual.
	// For now, we'll just release all resets.
	SYSCON->PRESETCTRL |= 0x07; //SYSCON_PRESETCTRL_I2C_RST_N;		

	// Turn AHB clock for peripherals: GPIO, IOCON and I2C
	SYSCON->SYSAHBCLKCTRL |= SYSCON_SYSAHBCLKCTRL_GPIO | SYSCON_SYSAHBCLKCTRL_IOCON | SYSCON_SYSAHBCLKCTRL_I2C;

	I2C->CONCLR = I2C_CONCLR_AAC | I2C_CONCLR_SIC | I2C_CONCLR_STAC | I2C_CONCLR_I2ENC; //clear flags

	I2C->SCLH = scl;
	I2C->SCLL = scl;

	NVIC_EnableInterrupt(NVIC_I2C0);	//enable I2C interrupt

	I2C->CONSET = I2C_CONSET_I2EN; //turn on I2C engine

}

void i2c_handler(void) {
	uint32_t status = 0xf8 & I2C->STAT;
	
	NVIC_ClearInterruptPending(NVIC_I2C0);

	I2C_STATUS userStatus = I2C_STATUS_OK; 	//status code to return

	switch (status) {
		case I2C_STAT_START_SENT:			//Start condition: load address+read/write, clear start
		case I2C_STAT_REP_START_SENT:
			I2C->DAT = (i2c_state->slaveAddress << 1) | ((i2c_state->toWrite > 0) ? 0 : 1);	//SLA+R/W
			I2C->CONCLR = I2C_CONCLR_STAC;
			break;
		case I2C_STAT_SLAW_ACKED:			//Write addressing succeeded
		case I2C_STAT_DATA_WRITE_ACKED:		//Byte successfully written
			if (i2c_state->toWrite > 0) {		//more to write
				I2C->DAT = *(i2c_state->writeBuffer);
				i2c_state->writeBuffer++;
				i2c_state->toWrite--;
			} else if (i2c_state->toRead > 0) {		//write done, have something to read
				I2C->CONSET = I2C_CONSET_STA;
			} else {						//nothing to read or write any more -> stop
				userStatus = I2C_STATUS_OK;
				goto stopAndNotifyUser;
			}
			break;
		case I2C_STAT_SLAW_NACKED:			//SLAW failed -> finish, notify
			i2c_state->toWrite = 0;
			i2c_state->toRead = 0;
			I2C->CONCLR = I2C_CONCLR_STAC | I2C_CONCLR_AAC;
			userStatus = I2C_STATUS_ADDRESSING_FAILED;
			goto stopAndNotifyUser;
			break;
		case I2C_STAT_DATA_WRITE_NACKED:	//Data write failed -> finish, notify
			i2c_state->toWrite = 0;
			i2c_state->toRead = 0;
			I2C->CONCLR = I2C_CONCLR_STAC | I2C_CONCLR_AAC;
			userStatus = I2C_STATUS_DATA_FAILED;
			goto stopAndNotifyUser;
			break;
		case I2C_STAT_ARBITRATION_LOST:		//Arbitration lost -> finish, notify
			i2c_state->toWrite = 0;
			i2c_state->toRead = 0;
			I2C->CONCLR = I2C_CONCLR_STAC | I2C_CONCLR_AAC;
			userStatus = I2C_STATUS_ARBITRATION_LOST;
			goto notifyUser;
			break;
		case I2C_STAT_SLAR_ACKED:
			if (i2c_state->toRead > 1) I2C->CONSET = I2C_CONSET_AA;
			break;
		case I2C_STAT_SLAR_NACKED:			//SLAR nacked -> finish, notify
			i2c_state->toWrite = 0;
			i2c_state->toRead = 0;
			I2C->CONCLR = I2C_CONCLR_STAC | I2C_CONCLR_AAC;
			userStatus = I2C_STATUS_ADDRESSING_FAILED;
			goto stopAndNotifyUser;
			break;
		case I2C_STAT_DATA_READ_ACKED:		//byte read and acked (more to come)
		case I2C_STAT_DATA_READ_NACKED:		//byte read and nacked (done)
			if (i2c_state->toRead) {
				uint8_t val = I2C->DAT;
				*(i2c_state->readBuffer) = val;
				i2c_state->readBuffer++;
				i2c_state->toRead--;
				if (i2c_state->toRead <= 1) I2C->CONCLR = I2C_CONCLR_AAC;
			}
			if (!i2c_state->toRead) {
				I2C->CONSET = I2C_CONSET_STO;
				userStatus = I2C_STATUS_OK;
				goto stopAndNotifyUser;
			}
			break;
		default: 
			break;
	}

	I2C->CONCLR = I2C_CONCLR_SIC; 	//Clear interrupt - continue bus
	return;

stopAndNotifyUser:
	I2C->CONSET = I2C_CONSET_STO;
notifyUser:
	I2C->CONCLR = I2C_CONCLR_SIC;

	if (i2c_state->completionHandler) i2c_state->completionHandler(i2c_state->refcon, userStatus);
}

I2C_STATUS I2C_Write(uint8_t addr,
         		    uint16_t len,
		            uint8_t* buf,
               		I2C_CompletionHandler handler,
               		uint32_t refcon) {
	return I2C_WriteRead(addr, len, buf, 0, NULL, handler, refcon);
}

I2C_STATUS I2C_Read(uint8_t addr,
	           		uint16_t len,
	           		uint8_t* buf,
	           		I2C_CompletionHandler handler,
	           		uint32_t refcon) {
	return I2C_WriteRead(addr, 0, NULL, len, buf, handler, refcon);
}

I2C_STATUS I2C_WriteRead(uint8_t addr,
						uint16_t writeLen,
						uint8_t* writeBuf,
						uint16_t readLen,
						uint8_t* readBuf,
						I2C_CompletionHandler handler,
						uint32_t refcon) {
	if (i2c_state == POINTER_NOT_SET) return I2C_STATUS_UNINITIALIZED;
	if (I2C_TransactionRunning()) return I2C_STATUS_BUSY;
	i2c_state->refcon = refcon;
	i2c_state->slaveAddress = addr;
	i2c_state->toWrite = writeLen;
	i2c_state->writeBuffer = writeBuf;
	i2c_state->toRead = readLen;
	i2c_state->readBuffer = readBuf;
	i2c_state->completionHandler = handler;
	I2C->CONSET = I2C_CONSET_STA; //send start condition.
	return I2C_STATUS_OK;
}

bool I2C_TransactionRunning() {
	return ((i2c_state->toRead > 0) || (i2c_state->toWrite > 0));
}
