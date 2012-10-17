#ifndef _HID_
#define _HID_

#include "usb.h"
#include "usbhidspec.h"

//forward declaration of USBHID_Behaviour_Struct
typedef struct USBHID_Behaviour_Struct USBHID_Behaviour_Struct;

/** called when a device-to-host report is requested. Implementations should
 * fill outBuffer with current values.
 * @param device USB device this call originated from
 * @param behaviour the behaviour this call originated from
 * @param reportType report type (1=input, 2=output, 3=feature)
 * @param reportId report id to fill. Currently, only one report (0) is
 * supported. Hi byte contains report type.
 * @return length of valid data */

typedef uint16_t (*HidInReportHandler)(USB_Device_Struct* device,
									   const USBHID_Behaviour_Struct* behaviour,
									   USB_HID_REPORTTYPE reportType,
									   uint8_t reportId);


/** called when a host-to-device report has arrived in inBuffer
 * @param device USB device this call originated from
 * @param behaviour the behaviour this call originated from
 * @param reportType report type
 * @param reportId report id. Currently, only one report (0) is supported.
 * @param len langth of report */

typedef void (*HidOutReportHandler)(USB_Device_Struct* device,
									const USBHID_Behaviour_Struct* behaviour,
									USB_HID_REPORTTYPE reportType,
									uint8_t reportId,
									uint16_t len);


/** our HID behaviour representation - state variables are added as
 *  pointers to RAM, so this struct may be const */

typedef struct USBHID_Behaviour_Struct {
	USB_Behaviour_Struct baseBehaviour;
	uint8_t interfaceNumber;
	const uint8_t* hidDescriptor;
	const uint8_t* reportDescriptor;
	uint16_t reportDescriptorLen;
	HidInReportHandler inReportHandler;
	HidOutReportHandler outReportHandler;
	uint8_t* inBuffer;
	uint8_t* outBuffer;
	uint8_t* idleValue;
	uint8_t* currentProtocol;
} USBHID_Behaviour_Struct;

/** may be called to push an IN report when inputs have changed. Will
 *  trigger the in report handler in the near
 *  future.
 *  @param device USB device to use @param behaviour behaviour to use
 *  @param reportType @param reportId report ID */

void USBHID_PushReport(USB_Device_Struct* device,
				   const USBHID_Behaviour_Struct* behaviour,
				   USB_HID_REPORTTYPE reportType,
				   uint8_t reportId);


/* USBHID base behaviour handlers. May be used to manually initialize a
 * USB_Behaviour_Struct at runtime,
 * for compile time initialization, you may use the
 * MAKE_USBHID_BASE_BEHAVIOUR macro. */

bool USBHID_ExtendedControlSetupHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

bool USBHID_EndpointDataHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour, uint8_t epIdx);

void USBHID_ConfigChangeHandler(USB_Device_Struct* device, const USB_Behaviour_Struct* behaviour);

#define MAKE_USBHID_BASE_BEHAVIOUR {\
	USBHID_ExtendedControlSetupHandler,\
	USBHID_EndpointDataHandler,\
	NULL,\
	NULL,\
	USBHID_ConfigChangeHandler\
}



#endif
