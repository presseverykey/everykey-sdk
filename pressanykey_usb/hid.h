#ifndef _HID_
#define _HID_

#include "../pressanykey/pressanykey.h"

typedef enum {
	HID_REPORTTYPE_INPUT = 1,
	HID_REPORTTYPE_OUTPUT = 2,
	HID_REPORTTYPE_FEATURE = 3
} HID_REPORTTYPE;

/** called when a device-to-host report is requested. Implementations should
 * fill outBuffer with current values.
 * @param reportType report type (1=input, 2=output, 3=feature)
 * @param reportId report id to fill. Currently, only one report (0) is
 * supported. Hi byte contains report type.
 * @return length of valid data */

typedef uint16_t (*HidInReportHandler)(HID_REPORTTYPE reportType, uint8_t reportId);


/** called when a host-to-device report has arrived in inBuffer
 * @param reportType report type
 * @param reportId report id. Currently, only one report (0) is supported.
 * @param len langth of report */

typedef void (*HidOutReportHandler)(HID_REPORTTYPE reportType, uint8_t reportId, uint16_t len);


/** initializes a HID device. For now, only simple HID devices are supported
 * (one HID interface, no other interfaces, one report) 
 * @param deviceDesc HID device descriptor
 * @param string1 string descriptor 1 (for use in other descriptors)
 * @param string2 string descriptor 2 (for use in other descriptors)
 * @param string2 string descriptor 3 (for use in other descriptors)
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

void HIDInit(	const uint8_t* deviceDesc,
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
				HidOutReportHandler outReportHandler);

/** may be called to push an IN report when inputs have changed 
 * @param reportType 
 * @param reportId report ID */

void HIDPushReport(HID_REPORTTYPE reportType, uint8_t reportId);


#endif
