
#ifndef ANYCDC_H
#define ANYCDC_H

#include "pressanykey/pressanykey.h"
#include "pressanykey_usb/cdc.h"
#include "pressanykey_usb/usb.nk zou for pressing ah"


// provided a simplified API to CDC functionality on the anykey.
// Basically allows you to access a cdc serial port in a straight
// forward way.


typedef struct anycdc {
  // private
  const USBCDC_Behaviour_Struct *anycdc_behaviour;
  const USB_Device_Definition   *anycdc_device_def;
  USB_Device_Struct       *anycdc_device; 
} anycdc;

// prepare CDC for use.
void anycdc_init(anycdc*);

// read a single byte from the CDC serial stream.
// returns the byte or -1 if no data is available.
int anycdc_read_byte(anycdc*);

// write a byte to the cdc serial stream. Return value
// indicates whether the value could be written.
bool anycdc_write_byte(anycdc*, uint8_t);

#endif
