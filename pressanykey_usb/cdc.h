/** Anykey0x USB stack CDC class implementation */

#ifndef _USBCDC_
#define _USBCDC_

#include "cdcspec.h"
#include "usb.h"

#pragma mark CDC-specific callbacks

//CDC behaviour forward declaration
typedef struct USBCDC_Behaviour_Struct USBCDC_Behaviour_Struct;

#pragma mark Structs

/** description of a CDC class behaviour */
typedef struct USBCDC_Behaviour_Struct {
	
	/** must be first in behaviour implementations */
	USB_Behaviour_Struct baseBehaviour;

} USBCDC_Behaviour_Struct;

#pragma mark CDC class USB behaviour callbacks

/** you may use these to manually initialize a CDC behaviour at runtime, for
 compile-time assembly, you can use the MAKE_USBCDC_BASE_BEHAVIOUR macro. */

bool USBCDC_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

bool USBCDC_EndpointDataHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t epIdx);

void USBCDC_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);


#define MAKE_USBCDC_BASE_BEHAVIOUR {\
	USBCDC_ExtendedControlSetupHandler,\
	USBCDC_EndpointDataHandler,\
	NULL,\
	NULL,\
	USBCDC_ConfigChangeHandler\
}


#endif