#include "cdc.h"

bool USBCDC_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
	return false;
}

bool USBCDC_EndpointDataHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t epIdx) {
	return true;
}

void USBCDC_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour) {
}
