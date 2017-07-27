#include "ssp.h"
#include "gpio.h"
#include "memorymap.h"

void SSP_Init(uint8_t clockDiv, uint8_t datasize, SSP_CR0_VALUES frameformat, bool idleClockHigh, bool dataOnSecond, bool master) {


	//Set SSP0_RST_N off and on to reset and run
	SYSCON->PRESETCTRL &= ~SYSCON_PRESETCTRL_SSP0_RST_N;
	SYSCON->PRESETCTRL |= SYSCON_PRESETCTRL_SSP0_RST_N;

	// Turn AHB clock for peripherals GPIO, IOCON and SSP
	SYSCON->SYSAHBCLKCTRL |= SYSCON_SYSAHBCLKCTRL_GPIO | SYSCON_SYSAHBCLKCTRL_IOCON | SYSCON_SYSAHBCLKCTRL_SSP;

	
	//Enable SSP clock
//	SYSCON->SSP0CLKDIV = 255;	//********
//	SSP0->CPSR = 254;			//********
	SYSCON->SSP0CLKDIV = 1;	//AHB clock is divided by 1 to get the SSP peripheral clock (clock on)
	SSP0->CPSR = 2*clockDiv;  //SSP peripheral clock is divided by this factor to get the prescaler clock

	//Set pin functions (0_8 -> MISO, 0_9 -> MOSI, 2_11 -> SCK)
	every_gpio_set_function(&(IOCON->PIO0_8), IOCON_IO_FUNC_PIO0_8_MISO, IOCON_IO_ADMODE_DIGITAL);
	every_gpio_set_function(&(IOCON->PIO0_9), IOCON_IO_FUNC_PIO0_9_MOSI, IOCON_IO_ADMODE_DIGITAL);
	

	//We use pin PIO2_11 for SSP clock
	IOCON->SCK0_LOC = IOCON_SCK0_LOC_PIO2_11;
	every_gpio_set_function(&(IOCON->PIO2_11), IOCON_IO_FUNC_PIO2_11_SCK, IOCON_IO_ADMODE_DIGITAL);

	uint32_t cr0 = (SSP_CR0_DSS_4BIT + (datasize - 4)) |
		frameformat	|
		(idleClockHigh ? SSP_CR0_CPOL_HIGH : SSP_CR0_CPOL_LOW) | 
		(dataOnSecond ? SSP_CR0_CPHA_SECOND : SSP_CR0_CPHA_FIRST) | 
//		(SSP_CR0_SCR_BASE * (clockDiv - 1));	
		(SSP_CR0_SCR_BASE * 64);		//***************

	//SSP prescaler clock is divided by this-1 to get baud rate
	//SSP clock is main clock / SSP0CLKDIV / CPSR / (SCR+1)
	
	uint32_t cr1 = SSP_CR1_LBM_NORMAL | SSP_CR1_SSE_ENABLE |
		(master ? SSP_CR1_MS_MASTER : SSP_CR1_MS_SLAVE);

	SSP0->CR0 = cr0;
	SSP0->CR1 = cr1;	//This call finally turns SSP on
	
	//Clear receive FIFO
	while (SSP0->SR & (SSP_SR_RNE | SSP_SR_BSY)) {
		uint32_t ignore = SSP0->DR;
	}
}

uint16_t SSP_Transfer(uint16_t value) {
	while ((SSP0->SR & (SSP_SR_TNF | SSP_SR_BSY)) != SSP_SR_TNF) {};	//wait until transfer fifo is not full
	SSP0->DR = value;
	while ((SSP0->SR & (SSP_SR_RNE | SSP_SR_BSY)) != SSP_SR_RNE) {};	//wait until receive fifo is not empty
	uint32_t read = SSP0->DR;
	return read;
}

