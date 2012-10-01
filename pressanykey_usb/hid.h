#ifndef _HID_
#define _HID_

#include "../pressanykey/pressanykey.h"
#include "usbhidspec.h"

//forward declaration of HID_Device_Struct
typedef struct HID_Device_Struct HID_Device_Struct;

/** called when a device-to-host report is requested. Implementations should
 * fill outBuffer with current values.
 * @param device reference to our device
 * @param reportType report type (1=input, 2=output, 3=feature)
 * @param reportId report id to fill. Currently, only one report (0) is
 * supported. Hi byte contains report type.
 * @return length of valid data */

typedef uint16_t (*HidInReportHandler)(HID_Device_Struct* device,
									   USB_HID_REPORTTYPE reportType,
									   uint8_t reportId);


/** called when a host-to-device report has arrived in inBuffer
 * @param device reference to our device
 * @param reportType report type
 * @param reportId report id. Currently, only one report (0) is supported.
 * @param len langth of report */

typedef void (*HidOutReportHandler)(HID_Device_Struct* device, 
									USB_HID_REPORTTYPE reportType,
									uint8_t reportId, uint16_t len);


/** our device representation - state and callbacks */
typedef struct HID_Device_Struct {
	USB_Device_Struct* usbDevice;

	uint8_t interfaceNumber;
	uint8_t currentProtocol;
	uint8_t* hidDescriptor;
	uint8_t* reportDescriptor;
	uint16_t reportDescriptorLen;
	uint8_t* inBuffer;
	uint8_t* outBuffer;
	HidInReportHandler inReportHandler;
	HidOutReportHandler outReportHandler;
	uint8_t idleValue;
	
} HID_Device_Struct;



/** initializes a HID device. For now, only simple HID devices are supported
 * (one HID interface, no other interfaces, one report) 
 * @param emptyUSBDevice pointer to an unitialized USB device struct in RAM
 * @param emptyHIDDevice pointer to an unitialized HID device struct in RAM
 * @param interfaceNum number of HID interface in the device
 * @param deviceDesc HID device descriptor
 * @param string1 string descriptor 1 (for use in other descriptors)
 * @param string2 string descriptor 2 (for use in other descriptors)
 * @param string3 string descriptor 3 (for use in other descriptors)
 * @param hidDesc USB HID descriptor (may point to repective subpart of
 *        configuration descriptor)
 * @param reportDesc USB HID report descriptor
 * @param reportDescLen length of HID report descriptor in bytes
 * @param inReportBuffer pointer to a buffer. Must be large enough for all IN
 *        reports
 * @param outReportBuffer pointer to a buffer. Must be large enough for all OUT
 *        reports
 * @param inReportHandler callback to fill IN reports
 * @param outReportHandler callback to fill OUT reports */

void HIDInit(	USB_Device_Struct* emptyUSBDevice,	
				HID_Device_Struct* emptyHIDDevice,
				int interfaceNum,
				const uint8_t* deviceDesc,
				const uint8_t* configDesc,
				const uint8_t* string1,
				const uint8_t* string2,
				const uint8_t* string3,
				const uint8_t* hidDesc,
				const uint8_t* reportDesc,
				uint16_t reportDescLen,
				uint8_t* inReportBuffer,
				uint8_t* outReportBuffer,
				HidInReportHandler inReportHandler,
				HidOutReportHandler outReportHandler
);

/** may be called to push an IN report when inputs have changed 
 * @param device device to push the report
 * @param reportType 
 * @param reportId report ID */

void HIDPushReport(HID_Device_Struct* device, USB_HID_REPORTTYPE reportType, uint8_t reportId);


#endif
