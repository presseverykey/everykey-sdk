
#ifndef EVERYCDC_H
#define EVERYCDC_H

#include "everykey/everykey.h"
#include "everykey_usb/cdc.h"
#include "everykey_usb/usb.h"


// provided a simplified API to CDC functionality on the everykey.
// Basically allows you to access a cdc serial port in a straight
// forward way.


typedef struct everycdc {
  // private
  const USBCDC_Behaviour_Struct *everycdc_behaviour;
  const USB_Device_Definition   *everycdc_device_def;
  USB_Device_Struct       *everycdc_device; 
} everycdc;

// prepare CDC for use.
void everycdc_init(everycdc*);

// read a single byte from the CDC serial stream.
// returns the byte or -1 if no data is available.
int everycdc_read_byte(everycdc*);

// write a byte to the cdc serial stream. Return value
// indicates whether the value could be written.
bool everycdc_write_byte(everycdc*, uint8_t);

#endif
