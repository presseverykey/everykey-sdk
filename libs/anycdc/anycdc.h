
#ifndef ANYCDC_H
#define ANYCDC_H

#include "pressanykey/pressanykey.h"
#include "pressanykey_usb/cdc.h"
#include "pressanykey_usb/usb.h"


// provided a simplified API to CDC functionality on the anykey.
// Basically allows you to access a cdc serial port in a straight
// forward way.


typedef struct anycdc {
  // private
  const USBCDC_Behaviour_Struct *anycdc_behaviour;
  const USB_Device_Definition   *anycdc_device_def;
  USB_Device_Struct       *anycdc_device; 
} anycdc;

void anycdc_init(anycdc*);

int anycdc_read_byte(anycdc*);

bool anycdc_write_byte(anycdc*, uint8_t);

#endif
