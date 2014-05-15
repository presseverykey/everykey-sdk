#ifndef _KEYBOARD_
#define _KEYBOARD_

#include "hid.h"

/** initializes a HID keyboard
 * @param deviceDefinition pointer to an uninitialized USB_Device_Definition in RAM
 * @param device pointer to an uninitialized USB_Device_Struct in RAM
 * @param hid pointer to an uninitialized USBHID_Behaviour_Struct in RAM
 * @param inBuffer pointer to a buffer in RAM. Must be large enough for all IN reports
 * @param outBuffer pointer to a buffer in RAM. Must be large enough for all OUT reports
 * @param idleValue pointer to a byte in RAM.
 * @param currentProtocol pointer to a byte in RAM.
 * @param inReportHandler callback to fill IN reports
 * @param outReportHandler callback to fill OUT reports */

void KeyboardInit(USB_Device_Definition* deviceDefinition,
    USB_Device_Struct* device,
    USBHID_Behaviour_Struct* hid,
    uint8_t* inBuffer, 
    uint8_t* outBuffer,
    uint8_t* idleValue,
    uint8_t* currentProtocol,
    HidInReportHandler inReportHandler, 
    HidOutReportHandler outReportHandler);
	
#endif
