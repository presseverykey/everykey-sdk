#ifndef _KEYBOARD_
#define _KEYBOARD_

#include "hid.h"

/** initializes a HID keyboard
 * @param emptyUSBDevice pointer to an uninitialized USB_Device_Struct in RAM
 * @param emptyHIDDevice pointer to an uninitialized HID_Device_Struct in RAM
 * @param inReportBuffer pointer to a buffer. Must be large enough for all IN reports
 * @param outReportBuffer pointer to a buffer. Must be large enough for all OUT reports
 * @param inReportHandler callback to fill IN reports
 * @param outReportHandler callback to fill OUT reports */

void KeyboardInit(USB_Device_Struct* emptyUSBDevice,
				  HID_Device_Struct* emptyHIDDevice,
				  uint8_t* inBuffer, 
                  uint8_t* outBuffer, 
                  HidInReportHandler inReportHandler, 
                  HidOutReportHandler outReportHandler);

#endif
