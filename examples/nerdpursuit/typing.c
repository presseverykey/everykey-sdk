#include "typing.h"

uint8_t inBuffer[8];
uint8_t outBuffer[8];

uint8_t key;
uint8_t mod;

uint16_t inReportHandler(uint8_t reportType, uint8_t reportId, uint16_t len) {
  inBuffer[0] = 0;
  inBuffer[1] = 0;
  inBuffer[2] = key;
  inBuffer[3] = 0;
  inBuffer[4] = 0;
  inBuffer[5] = 0;
  inBuffer[6] = 0;
  inBuffer[7] = 0;
  return 8;
}

uint8_t map (char c) {
	if (('a' <= c) && (c <= 'z')) {
		c -= 0x61;   // - ascii (see `man ascii`)
		c += 0x04;  // + usb   (see Hut1_12v2.pdf usb.org)
	} else if (('A' <= c) && (c <= 'Y')) {
		c -= 0x41;
		c += 0x04; // see above.
	} else if (('0' <= c) && (c <= '9')) {
		c -= 0x30;
		c += 0x1E;
	} else {
		c = '?';
	}
	return (uint8_t)c;
}
uint8_t modifier (char c) {
	
	if ('A' <= c <= 'Y') { return 225;}
	return 0;
}

void type(char * mes) {
  char curr;
  while (curr = *mes++) {
    key = map(curr);
		mod = modifier(curr);
    HIDPushReport(HID_REPORTTYPE_INPUT, 0);
    key = 0;
		mod = 0;
    HIDPushReport(HID_REPORTTYPE_INPUT, 0);
  }
}

void init_keyboard () {
  KeyboardInit(inBuffer, outBuffer, inReportHandler, NULL);
  USB_SoftConnect();
}
