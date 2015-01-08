/* Universal asynchronous receiver transmitter functions - serial port */

#ifndef _UART_
#define _UART_

#include "types.h"

#define UART_RXD_PORT 1
#define UART_RXD_PIN 6

#define UART_TXD_PORT 1
#define UART_TXD_PIN 7

#define UART_CTS_PORT 0
#define UART_CTS_PIN 7

#define UART_RTS_PORT 1
#define UART_RTS_PIN 5

/** combined values for parity enable and parity mode */
typedef enum {
    UART_PARITY_NONE = 0x00,
    UART_PARITY_ODD = 0x08,
    UART_PARITY_EVEN = 0x18,
    UART_PARITY_ONE = 0x28,
    UART_PARITY_ZERO = 0x38
} UART_Parity;

/** UART status codes */
typedef enum {
    UART_STATUS_DATA_AVAILABLE = 1, //Data to read available - call UART_Read
    UART_STATUS_CAN_SEND_MORE,      //Can send more bytes - can call UART_Write
    UART_STATUS_OVERRUN,            //Receive buffer overrun
    UART_STATUS_PARITY_ERROR,       //Parity error encountered
    UART_STATUS_FRAMING_ERROR,      //0 stop bit received
    UART_STATUS_BREAK               //Break condition detected
} UART_Status;

/** User-supplied handler for errors or other status events
@param status uart status */
typedef void (*UART_StatusHandler)(UART_Status status);

/** Internal UART software driver state */
typedef struct {
    UART_StatusHandler statusHandler;
} UART_State;

/** Initializes the UART peripheral to a specific mode. This call currently assumes 72 MHz main clock.
@param baud baud rate (will try use closest value)
@param dataBits number of data bits (5..8)
@param parity parity
@param stopBits (1 or 2 - 2 becomes 1.5 when dataBits = 5) 
@param useHWFlow true to enable hardware flow control (RTS/CTS), false otherwise
@param statusHandler user-supplied handler for status changes or events - may be NULL */

void UART_Init( uint32_t baud,
                uint8_t dataBits,
                UART_Parity parity,
                uint8_t stopBits,
                bool useHWFlow,
                UART_StatusHandler statusHandler);

/** Initializes the UART peripheral to a specific mode. This call currently assumes 72 MHz main clock.
@param baud baud rate (will try use closest value)
@param dataBits number of data bits (5..8)
@param parity parity
@param stopBits (1 or 2 - 2 becomes 1.5 when dataBits = 5) 
@param useHWFlow true to enable hardware flow control (RTS/CTS), false otherwise
@param statusHandler user-supplied handler for status changes or events - may be NULL 
@param threshold one of the UART_FCR interrupt generation threshold values (UART_FCR_RXTLVL1/4/8/14) */
void UART_Init_Ext( uint32_t baud,
                    uint8_t dataBits,
                    UART_Parity parity,
                    uint8_t stopBits,
                    bool useHWFlow,
                    UART_StatusHandler statusHandler,
                    uint8_t threshold);

/** adds bytes to the send buffer. Note that this function does not use a separate FIFO buffer, so writing space is very limited.
@param buffer bytes to send
@param length length to send (1..16)
@return number of bytes that were added to the buffer */
uint8_t UART_Write(const uint8_t* buffer, uint8_t length);

/** reads bytes from the receive buffer.
@param buffer buffer to hold received data. specify nil to ignore data
@param maxLength length to send (1..16)
@return number of bytes that could be read */
uint8_t UART_Read(uint8_t* buffer, uint8_t maxLength);

/** reads one byte from the receive buffer.
@param buffer buffer to hold received data or nil to ignore data
@return true if data could be read */
bool UART_Read1(uint8_t* buffer);

/** starts transmitting a break condition (tx low) */
void UART_StartBreak();

/** stops transmitting a break condition (tx low) */
void UART_StopBreak();


//---------------------------
#pragma mark Low level access
//---------------------------

typedef struct {
    HW_RW RBR_THR_DLL;  //Receiver Buffer (DLAB=0 read), Transmitter holding register (DLAB=0 write) or Divisor Latch LSB (DLAB=1)
    HW_RW DLM_IER;      //Divisor Latch MSB (DLAB=1) or Interrupt enable (DLAB=0)
    HW_RW IIR_FCR;      //Interrupt ID register (read) or FIFO control register (write)
    HW_RW LCR;          //Line control register
    HW_RW MCR;          //Modem control register
    HW_RO LSR;          //Line status register
    HW_RO MSR;          //Modem status register
    HW_RW SCR;          //Scratch pad register
    HW_RW ACR;          //Auto-baud Control register
    HW_RS RESERVED1;    
    HW_RW FDR;          //Fractional Divider register
    HW_RS RESERVED2;
    HW_RW TER;          //Transmit enable register
    HW_RS RESERVED3[6];
    HW_RW RS485CTRL;    //RS485 control
    HW_RW RS485ADRMATCH;//RS485 address match
    HW_RW RS485DLY;     //RS485 direction control delay
} UART_STRUCT;

#define UART_HW ((UART_STRUCT*)(0x40008000))

/* bits for tie IE register */
typedef enum {
    UART_IE_RDR = 0x01,             //Receive data available
    UART_IE_THRE = 0x02,            //Transmitter holding register empty
    UART_IE_RXL = 0x04              //RX line status changed
} UART_IE;

/** bits and masks for the LCR register */
typedef enum {
    UART_LCR_DATABITS_MASK = 0x03,  //Data bits - 5
    UART_LCR_LONG_STOP = 0x04,      //1 = 2 or 1.5 stop bits (depending on databits), 0 = 1 stop bit
    UART_LCR_PARITY_MASK = 0x38,    //Parity enable and mode, see UART_PARITY for pre-shifted values
    UART_LCR_BC = 0x40,             //Break control. 1 = Send break
    UART_LCR_DLAB = 0x80            //1: access divider latch registers, 0 = other register functions
} UART_LCR;

/** bits for the MCS register */
typedef enum {
    UART_MCR_LMS = 0x01,            //Loopback mode select (usually diagnostics only)
    UART_MCR_RTSEN = 0x02,          //Enable Auto-RTS
    UART_MCR_CTSEN = 0x04           //Enable Auto-CTS
} UART_MCS;

/** values for the FIFO control register */
typedef enum {
    UART_FCR_FIFOEN = 0x01,         //FIFO enable (must be 1 for proper operation)
    UART_FCR_RXFIFOR = 0x02,        //RX FIFO reset
    UART_FCR_TXFIFOR = 0x04,        //TX FIFO reset
    UART_FCR_RXTLVL1 = 0x00,        //Interrupt threshold: 1 byte in RX buffer
    UART_FCR_RXTLVL4 = 0x40,        //Interrupt threshold: 4 bytes in RX buffer
    UART_FCR_RXTLVL8 = 0x80,        //Interrupt threshold: 8 bytes in RX buffer
    UART_FCR_RXTLVL14 = 0xc0,       //Interrupt threshold: 14 bytes in RX buffer
} UART_FCR;

/** values for Line Status Register (LSR) */
typedef enum {
    UART_LS_RDR = 0x01,             //Receiver data ready - 1 = can read data
    UART_LS_OE = 0x02,              //Overflow error
    UART_LS_PE = 0x04,              //Parity error
    UART_LS_FE = 0x08,              //Framing error
    UART_LS_BI = 0x10,              //Break interrupt
    UART_LS_THRE = 0x20,            //Transmitter holding register empty
    UART_LS_TEMT = 0x40,            //Transmitter empty
    UART_LS_RXFE = 0x80             //Error in RX FIFO - OE,PE,FE or BI
} UART_LSR;

#endif
