/** This application illustrates usage of the I2C interface. It reads a ams TCS3471 RGB color sensor
and turns the LED on and off based on the measured brightness. This example requires a TCS3471
connected to the I2C port (SCL = PIO0_4, SDA = PIO0_5). Add 3.3V pullup resistors to both lines.
See TCS3471 datasheet and NXP's I2C spec and notes for details. */

#include "anykey/anykey.h"

#define LED_PORT 0
#define LED_PIN 7
#define KEY_PORT 0
#define KEY_PIN 1

void sleep(uint32_t len) {
    volatile uint32_t val = 0;
    while (val < len) {
        val++;
    }
}

//--------------------
//--- Blocking I2C ---
//--------------------

/** the following block implements blocking I2C transactions and retries. The API is asynchronous,
calls a callback when a transaction finished (on success and failure). The blocking approach is
not very elegant but simple to use. Note that we need to timeout to avoid blocking the processor -
Adjust the defined values depending on your peripheral's speed and transfer length. */

#define I2C_START_MAX_RETRIES 1000
#define I2C_FINISH_MAX_RETRIES 1000000

/** variable to carry back the transaction result from callback interrupt to main thread */
volatile int i2cSyncStatus = 0;

/** callback when a I2C transaction succeeded */
void I2C_SyncReadCompletionHandler(uint32_t refcon, I2C_STATUS status) {
	i2cSyncStatus = status;
}

/** Blocking I2C transaction with retries */
I2C_STATUS I2C_WriteReadSync( uint8_t addr,
							uint16_t writeLen,
							uint8_t* writeBuf,
							uint16_t readLen,
							uint8_t* readBuf) {
	i2cSyncStatus = -1;
	int retries;
	I2C_STATUS status;

	//Try to start transaction
	for (retries = 0; retries < I2C_START_MAX_RETRIES; retries++) {
		status = I2C_WriteRead(addr, writeLen, writeBuf, readLen, readBuf, I2C_SyncReadCompletionHandler, 0);
		if (status == I2C_STATUS_OK) break;
	}		

	//If start succeeded, wait for finish
	if (status == I2C_STATUS_OK) {
		status = I2C_STATUS_TIMEOUT;	//return this if we didn't finish
		for (retries = 0; retries < I2C_FINISH_MAX_RETRIES; retries++) {
			if (i2cSyncStatus >= 0) {
				status = (I2C_STATUS)i2cSyncStatus;
				break;
			}
		}
	}
	return status;
}

//-----------------------------
//--- TCS3417-specific code ---
//-----------------------------

/** TCS3471 register set */
typedef enum {
	TCS3471_ENABLE  = 0x00,	//State and interrupt enable (RW)
	TCS3471_ATIME   = 0x01,	//ARBC ADC time (RW)
	TCS3471_WTIME   = 0x03,	//Wait time (RW)
	TCS3471_AILTL   = 0x04,	//RGBC interupt low thres low byte (RW)
	TCS3471_AILTH   = 0x05,	//RGBC interupt low thres high byte (RW)
	TCS3471_AIHTL   = 0x06,	//RGBC interupt high thres low byte (RW)
	TCS3471_AIHTH   = 0x07,	//RGBC interupt high thres high byte (RW)
	TCS3471_PERS    = 0x0C,	//Interrupt persistence register (RW)
	TCS3471_CONFIG  = 0x0D,	//Configuration (RW)
	TCS3471_CONTROL = 0x0F,	//Gain control register (RW)
	TCS3471_ID      = 0x12,	//Device ID (R)
	TCS3471_STATUS  = 0x13,	//Device status (R)
	TCS3471_CDATA   = 0x14, //Clear ADC low data register (R)
	TCS3471_CDATH   = 0x15, //Clear ADC high data register (R)
	TCS3471_RDATA   = 0x16, //Red ADC low data register (R)
	TCS3471_RDATH   = 0x17, //Red ADC high data register (R)
	TCS3471_GDATA   = 0x18, //Green ADC low data register (R)
	TCS3471_GDATH   = 0x19, //Green ADC high data register (R)
	TCS3471_BDATA   = 0x1a, //Blue ADC low data register (R)
	TCS3471_BDATH   = 0x1b, //Blue ADC high data register (R)
} TCS3471_REGISTER;

#define TCS3471_I2C_ID 0x29

/** generic read from TCS3471 register space, arbitrary length */
I2C_STATUS TCS3471_ReadRegisters(TCS3471_REGISTER startReg, int len, uint8_t* buf) {
	uint8_t cmd = 0xa0 | startReg;
	return I2C_WriteReadSync(TCS3471_I2C_ID, 1, &cmd, len, buf);
}

/** generic 1 byte register read from TCS3471. Returns 0 on failure. */
uint8_t TCS3471_ReadRegister(TCS3471_REGISTER reg) {
	uint8_t buf = 0;
	I2C_STATUS status = TCS3471_ReadRegisters(reg, 1, &buf);
	if (status != I2C_STATUS_OK) buf = 0;	//failed
	return buf;
}

/** generic write to a TCS3471 8 bit register */
I2C_STATUS TCS3471_WriteRegister(TCS3471_REGISTER reg, uint8_t value) {
	uint8_t buf[] = { 0xa0 | reg, value};
	I2C_STATUS status = I2C_WriteReadSync( TCS3471_I2C_ID, 2, buf, 0, NULL);
	return status;
}

/** turns on the chip (initially in sleep mode) and activates ADC conversion */
bool TCS3471_PowerOn() {
	I2C_STATUS status = TCS3471_WriteRegister(TCS3471_ENABLE, 0x01);
	if (status != I2C_STATUS_OK) return false;
	sleep(100000);
	status = TCS3471_WriteRegister(TCS3471_ENABLE, 0x03);
	return (status == I2C_STATUS_OK);
}

/** Adjust integration time (= exposure time) */
bool TCS3471_SetIntegrationTimeMS(uint16_t ms) {
	ms = ((ms * 10) / 24);
	if (ms > 0xff) ms = 0xff;
	uint8_t val = 0xff-(uint8_t)ms;
	I2C_STATUS status = TCS3471_WriteRegister(TCS3471_ATIME, val);
	return (status == I2C_STATUS_OK);
}

/** Adjust delay between samples */
bool TCS3471_SetWaitTimeMS(uint16_t ms) {
	ms = ((ms * 10) / 24);
	if (ms > 0xff) ms = 0xff;	//TODO: Set WLONG if needed
	uint8_t val = (uint8_t)ms;
	I2C_STATUS status = TCS3471_WriteRegister(TCS3471_WTIME, val);
	return (status == I2C_STATUS_OK);
}

/** returns the chip ID (should be 0x14 or 0x1d) */
uint8_t TCS3471_GetId() {
	return TCS3471_ReadRegister(TCS3471_ID);
}

/** returns the chip status register (i.e. sample valid) */
uint8_t TCS3471_GetStatus() {
	return TCS3471_ReadRegister(TCS3471_STATUS);
}

/** reads the sampled crgb (c=clear) brightness values */
bool TCS3471_GetColors(uint16_t* c, uint16_t* r, uint16_t* g, uint16_t* b) {
	uint8_t status = TCS3471_GetStatus();
	if (!(status & 1)) return false; //comm error or no valid sample
	uint8_t val[2] = {0,0};
	if (c) {
		I2C_STATUS status = TCS3471_ReadRegisters(TCS3471_CDATA, 2, val);
		if (status != I2C_STATUS_OK) return false;	//comm error
		*c = val[1]<<8 | val[0];
	}
	if (r) {
		I2C_STATUS status = TCS3471_ReadRegisters(TCS3471_RDATA, 2, val);
		if (status != I2C_STATUS_OK) return false;	//comm error
		*r = val[1]<<8 | val[0];
	}
	if (g) {
		I2C_STATUS status = TCS3471_ReadRegisters(TCS3471_GDATA, 2, val);
		if (status != I2C_STATUS_OK) return false;	//comm error
		*g = val[1]<<8 | val[0];
	}
	if (b) {
		I2C_STATUS status = TCS3471_ReadRegisters(TCS3471_BDATA, 2, val);
		if (status != I2C_STATUS_OK) return false;	//comm error
		*b = val[1]<<8 | val[0];
	}
	return true;
}


I2C_State i2cState;

void main () {
	sleep(100000); //For some reason, this seems to be a good thing to do

	GPIO_SetDir (LED_PORT, LED_PIN, GPIO_Output);
	GPIO_WriteOutput (LED_PORT, LED_PIN, false);
	GPIO_SetDir (KEY_PORT, KEY_PIN, GPIO_Input);
	GPIO_SETPULL (KEY_PORT, KEY_PIN, IOCON_IO_PULL_UP);

	//Start I2C in slow mode. Just pass in an empty I2C_Struct - used by I2C engine.
	I2C_Init(I2C_MODE_STANDARD, &i2cState); 
	
	if (!TCS3471_PowerOn()) {
		GPIO_WriteOutput(0,7,true);
		while (1) {}
	}
	
	if (!TCS3471_SetIntegrationTimeMS(100)) {
		GPIO_WriteOutput(0,7,true);
		while (1) {}
	}

	if (!TCS3471_SetWaitTimeMS(300)) {
		GPIO_WriteOutput(0,7,true);
		while (1) {}
	}

	while (1) {
		uint16_t r,g,b,c;
		if (TCS3471_GetColors(&c,&r,&g,&b)) {
			GPIO_WriteOutput(0,7,c > 1000);
		}
		sleep(100000);
	}
}
