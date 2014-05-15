// Structures and values taken from the USB HID spec. This is rather platform independent

#ifndef _USBHIDSPEC_
#define _USBHIDSPEC_

/** class-specific request codes */ 
typedef enum USB_HID_REQUEST {
	USB_REQ_HID_GETREPORT		= 0x01,
	USB_REQ_HID_GETIDLE			= 0x02,
	USB_REQ_HID_GETPROTOCOL		= 0x03,
	USB_REQ_HID_SETREPORT		= 0x09,
	USB_REQ_HID_SETIDLE			= 0x0a,
	USB_REQ_HID_SETPROTOCOL		= 0x0b
} USB_HID_REQUEST;

/** class specific descriptor types */
typedef enum USB_HID_DESCRIPTOR_TYPE {
	USB_DESC_HID_HID					= 0x21,
	USB_DESC_HID_REPORT					= 0x22,
	USB_DESC_HID_PHYSICAL				= 0x23
} USB_HID_DESCRIPTOR_TYPE;

/** report types as used in GetReport or SetReport - see USB HID spec 1.11, section 7.2.1 */
typedef enum USB_HID_REPORTTYPE {
	USB_HID_REPORTTYPE_INPUT = 1,
	USB_HID_REPORTTYPE_OUTPUT = 2,
	USB_HID_REPORTTYPE_FEATURE = 3
} USB_HID_REPORTTYPE;


#endif
