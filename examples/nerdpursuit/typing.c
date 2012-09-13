#include "typing.h"
#include "pressanykey/pressanykey.h"

#define LED_PORT 0
#define LED_PIN  7
uint8_t inBuffer[8];
uint8_t outBuffer[8];

uint8_t key;
uint8_t mod;

uint16_t inReportHandler(uint8_t reportType, uint8_t reportId) {
  inBuffer[0] = mod;
  inBuffer[1] = 0;
  inBuffer[2] = key;
  inBuffer[3] = 0;
  inBuffer[4] = 0;
  inBuffer[5] = 0;
  inBuffer[6] = 0;
  inBuffer[7] = 0;
  return 8;
}

void outReportHandler (uint8_t reportType, uint8_t reportId, uint16_t len) {}

uint8_t map (char c) {
	if (('a' <= c) && (c <= 'z')) {
		// c -= 0x61;   // - ascii (see `man ascii`)
		// c += 0x04;  // + usb   (see Hut1_12v2.pdf usb.org)
    return c - 0x5D;
	} else if (('A' <= c) && (c <= 'Z')) {
		//c -= 0x41;
		//c += 0x04; // see above.
    return c - 0x3D;
	} else if (('1' <= c) && (c <= '9')) {
	  //	c -= 0x31;
	  //	c += 0x1E;
    return c - 0x13;
	} else {
    switch (c) {
      case ' ': return 0x2C;
      case '\n': return 0x28;
      case '#': return 0x20;
      case '!': return 0x1e;
      case '0': return 0x27;
      // TODO various punctuation and brackets ...
      default: return 0x2D; // '-'
    }
	}
	return (uint8_t)0x2D;
}

uint8_t modifier (char c) {
  if (('A' <= c) && (c <= 'Z')) {	return 0x02;}
  switch(c) {
    case '#':
    case '!':
      return 0x02;
  }
	return 0;
}
void delay (int i) {
  volatile int j;
  for (j=0; j!=i; ++j) {}
}


void type(char * mes) {
  command("# ");
  command(mes);

}
void command (char * mes) {
  char curr;
  while (curr = *mes++, curr !=0) {
    key = map(curr);
		mod = modifier(curr);
    HIDPushReport(HID_REPORTTYPE_INPUT, 0);
    delay(50000);
    key = 0;
		mod = 0;
    HIDPushReport(HID_REPORTTYPE_INPUT, 0);
    delay(50000);
  }
}

void init_keyboard () {
  KeyboardInit(inBuffer, outBuffer, inReportHandler, outReportHandler);
  USB_SoftConnect();
}
