#ifndef _KEYBOARD_
#define _KEYBOARD_

#include "hid.h"

/** initializes a HID keyboard
 * @param inReportBuffer pointer to a buffer. Must be large enough for all IN reports
 * @param outReportBuffer pointer to a buffer. Must be large enough for all OUT reports
 * @param inReportHandler callback to fill IN reports
 * @param outReportHandler callback to fill OUT reports */
void KeyboardInit(uint8_t* inBuffer, uint8_t* outBuffer, HidInReportHandler inReportHandler, HidOutReportHandler outReportHandler);



#endif
