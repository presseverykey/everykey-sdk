#include "memorymap.h"
#include "uart.h"
#include "gpio.h"
#include "nvic.h"

/* By default, we receive data in blocks of this size (incomplete blocks are sent with a bit delay).
 This value is a compromise of interrupt overhead, transmission granularity and receive
 buffer reserve (16 bytes total). Allowed values: 1,4,8,14 */
#define RX_TLVL UART_FCR_RXTLVL8


UART_StatusHandler uartStatusHandler = NULL;


void UART_Init( uint32_t baud,
                uint8_t dataBits,
                UART_Parity parity,
                uint8_t stopBits,
                bool useHWFlow,
                UART_StatusHandler statusHandler) {
	UART_Init_Ext(baud, dataBits, parity, stopBits, useHWFlow, statusHandler,RX_TLVL);
}

void UART_Init_Ext( uint32_t baud,
                    uint8_t dataBits,
                    UART_Parity parity,
                    uint8_t stopBits,
                    bool useHWFlow,
                    UART_StatusHandler statusHandler,
                    UART_FCR threshold) {

	uartStatusHandler = statusHandler;

	/* Find best divider, fraction add and mul by trying all add/mul pairs (~100 tries)
	deriving the best divider and minimizing the difference between hypothetical and real pclk.
	uart = pclk / (16*div*(1+(mul/mul))
	uart = pclk / (16*div*(mul+add)/mul)
	baud16 := 16 * uart
	baud16 = pclk / (div*(mul+add)/mul)
	div*(mul+add)/mul = pclk / baud16
	div*(mul+add) = (pclk * mul) / baud16
	div = (pclk * mul) / (baud16 * (mul+add))
	pclk = baud16 * div * (mul+add) /mul */

	uint32_t bestMul = 1;
	uint32_t bestAdd = 0;
	uint32_t bestDiv = 1;
	uint32_t bestDiff = 0xffffffff;
	uint32_t baud16 = 16 * baud;

	uint32_t pclk = 72000000;	//72 MHz
	uint32_t mul;
	for (mul = 1; mul<=15; mul++) {
		uint32_t add;
		for (add = 0; add<mul; add++) {
			uint32_t upper = pclk * mul;
			uint32_t lower = baud16 * (mul+add);
			uint32_t div = (upper + (lower/2)) / lower;
			if ((add > 0) && (div < 3)) continue;
			uint32_t pclkHyp = baud16 * div * (mul+add) / mul;
			uint32_t diff = (pclkHyp > pclk) ? (pclkHyp - pclk) : (pclk - pclkHyp);
			if (diff < bestDiff) {
				bestDiff = diff;
				bestMul = mul;
				bestAdd = add;
				bestDiv = div;
			}
		}
	}

	uint8_t lineControlVal = ((dataBits-5) & (UART_LCR_DATABITS_MASK)) |
		((stopBits>1) ? UART_LCR_LONG_STOP : 0) |
		(parity & UART_LCR_PARITY_MASK);


	// Turn AHB clock for peripherals: GPIO and IOCON 
	SYSCON->SYSAHBCLKCTRL |= SYSCON_SYSAHBCLKCTRL_GPIO | SYSCON_SYSAHBCLKCTRL_IOCON;

	EVERY_GPIO_SET_FUNCTION(UART_TXD_PORT, UART_TXD_PIN, TXD, IOCON_IO_ADMODE_DIGITAL);
	EVERY_GPIO_SET_FUNCTION(UART_RXD_PORT, UART_RXD_PIN, RXD, IOCON_IO_ADMODE_DIGITAL);
	if (useHWFlow) {
		EVERY_GPIO_SET_FUNCTION(UART_CTS_PORT, UART_CTS_PIN, CTS, IOCON_IO_ADMODE_DIGITAL);
		EVERY_GPIO_SET_FUNCTION(UART_RTS_PORT, UART_RTS_PIN, RTS, IOCON_IO_ADMODE_DIGITAL);
	}

	// Turn AHB clock for UART
	SYSCON->SYSAHBCLKCTRL |= SYSCON_SYSAHBCLKCTRL_UART;

	//Set clock and timing	
	SYSCON->UARTCLKDIV = 1; 						// UART_CLK = PCLK / 1
	UART_HW->LCR = lineControlVal | UART_LCR_DLAB; 	//Access divider latch to set clocking data
	UART_HW->RBR_THR_DLL = bestDiv & 0xff;
	UART_HW->DLM_IER = (bestDiv >> 8) & 0xff;
	UART_HW->FDR = bestAdd | (bestMul << 4);
	UART_HW->LCR = lineControlVal;					//Turn DLAB to 0 again

	//Enable CTS / RTS if user requested it
	UART_HW->MCR = useHWFlow ? UART_MCR_CTSEN | UART_MCR_RTSEN : 0;

	//Enable and reset FIFOs (reset if FIFOs were already on)
	UART_HW->IIR_FCR = UART_FCR_FIFOEN | UART_FCR_RXFIFOR | UART_FCR_TXFIFOR | threshold;

	//TODO: Resetting the FIFOs should also clear them. Do we need to manually flush them here?

	//Enable interrupts
	UART_HW->DLM_IER = UART_IE_RDR | UART_IE_THRE | UART_IE_RXL; //enable all UART interrupt sources
 	
	NVIC_EnableInterrupt(NVIC_UART);
}

uint8_t UART_Write(const uint8_t* buffer, uint8_t length) {
	uint8_t written;
	for (written = 0; written < length; written++) {
		uint8_t lineStatus = UART_HW->LSR;
		if (!(lineStatus & UART_LS_THRE)) return written;
		UART_HW->RBR_THR_DLL = buffer[written];
	}
	return written;
}

uint8_t UART_Read(uint8_t* buffer, uint8_t maxLength) {
	uint8_t read;
	for (read = 0; read < maxLength; read++) {
		uint8_t lineStatus = UART_HW->LSR;
		if (!(lineStatus & UART_LS_RDR)) return read;
		if (buffer) {
			buffer[read] = UART_HW->RBR_THR_DLL;
		} else {
			UART_HW->RBR_THR_DLL;
		}
	}
}

/** starts transmitting a break condition (tx low) */
void UART_StartBreak() {
	UART_HW->LCR |= UART_LCR_BC;
}

/** stops transmitting a break condition (tx low) */
void UART_StopBreak() {
	UART_HW->LCR &= ~UART_LCR_BC;
}


void uart_handler() {
	uint8_t interruptReason = (UART_HW->IIR_FCR) & 0xf;	//both status and id
	switch (interruptReason) {
		case 0x00:		//Auto-baud interrupt
			//Do nothing
			break;
		case 0x01:		//Modem interrupt - Prio 4 (lowest)
			//Do nothing
			break;
		case 0x02:		//THRE (Transmitter holding register empty), prio 3, can write TX again
			if (uartStatusHandler) uartStatusHandler(UART_STATUS_CAN_SEND_MORE);
			break;
		case 0x06:		//Receive Line Status (Prio 1)
			{
				uint8_t lineStatus = UART_HW->LSR;
				if (uartStatusHandler) {
					if (lineStatus & UART_LS_OE) uartStatusHandler(UART_STATUS_OVERRUN);
					if (lineStatus & UART_LS_PE) uartStatusHandler(UART_STATUS_PARITY_ERROR);
					if (lineStatus & UART_LS_FE) uartStatusHandler(UART_STATUS_FRAMING_ERROR);
					if (lineStatus & UART_LS_BI) uartStatusHandler(UART_STATUS_BREAK);
				}
			}
			break;
		case 0x04:		//Receive Data Available (Prio 2) - RX threshold reached
			if (uartStatusHandler) uartStatusHandler(UART_STATUS_DATA_AVAILABLE);
			break;
		case 0x0c:		//Character Timeout (Prio 2) - RX data and no new data for some time
			if (uartStatusHandler) uartStatusHandler(UART_STATUS_DATA_AVAILABLE);
			break;
		default:	//All other values are reserved
			break;
	}
}