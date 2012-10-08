// Structures and values taken from the usb spec. This is rather platform independent

#ifndef _USBSPEC_
#define _USBSPEC_

typedef struct USB_Setup_Packet {
	uint8_t		bmRequestType;
	uint8_t		bRequest;
	uint8_t		wValueL;
	uint8_t		wValueH;
	uint8_t		wIndexL;
	uint8_t		wIndexH;
	uint8_t		wLengthL;
	uint8_t		wLengthH;
} __attribute__((packed)) USB_Setup_Packet;

typedef enum USB_REQUEST_TYPE {
	USB_RT_DIR_MASK				= 0x80,
	USB_RT_DIR_HOST_TO_DEVICE	= 0x00,
	USB_RT_DIR_DEVICE_TO_HOST	= 0x80,
	USB_RT_TYPE_MASK			= 0x60,
	USB_RT_TYPE_STANDARD		= 0x00,
	USB_RT_TYPE_CLASS			= 0x20,
	USB_RT_TYPE_VENDOR			= 0x40,
	USB_RT_RECIPIENT_MASK		= 0x1f,
	USB_RT_RECIPIENT_DEVICE		= 0x00,
	USB_RT_RECIPIENT_INTERFACE	= 0x01,
	USB_RT_RECIPIENT_ENDPOINT	= 0x02,
	USB_RT_RECIPIENT_OTHER		= 0x03
} USB_REQUEST_TYPE;

typedef enum USB_REQUEST {
	USB_REQ_GET_STATUS			= 0x00,
	USB_REQ_CLEAR_FEATURE		= 0x01,
	USB_REQ_SET_FEATURE			= 0x03,
	USB_REQ_SET_ADDRESS			= 0x05,
	USB_REQ_GET_DESCRIPTOR		= 0x06,
	USB_REQ_SET_DESCRIPTOR		= 0x07,
	USB_REQ_GET_CONFIGURATION	= 0x08,
	USB_REQ_SET_CONFIGURATION	= 0x09,
	USB_REQ_GET_INTERFACE		= 0x0a,
	USB_REQ_SET_INTERFACE		= 0x0b
} USB_REQUEST;

typedef enum USB_DESCRIPTOR_TYPE {
	USB_DESC_DEVICE						= 0x01,
	USB_DESC_CONFIGURATION				= 0x02,
	USB_DESC_STRING						= 0x03,
	USB_DESC_INTERFACE					= 0x04,
	USB_DESC_ENDPOINT					= 0x05,
	USB_DESC_DEVICE_QUALIFIER			= 0x06,
	USB_DESC_OTHER_SPEED_CONFIGURATION	= 0x07,
	USB_DESC_INTERFACE_POWER			= 0x08
} USB_DESCRIPTOR_TYPE;

/** possible attributes of USB configurations */
typedef enum USB_CONFIGURATION_ATTRIBUTE_FLAGS {
	USB_CONFIGURATION_SELFPOWERED		= 0x40,
	USB_CONFIGURATION_REMOTE_WAKEUP		= 0x20
} USB_CONFIGURATION_ATTRIBUTE_FLAGS;

/** feature IDs for devices, interfaces and endpoints */
typedef enum USB_FEATURE_ID {
	USB_FEATURE_ENDPOINT_HALT			= 0x00,
	USB_FEATURE_DEVICE_REMOTE_WAKEUP	= 0x01,
	USB_FEATURE_TEST_MODE				= 0x02
} USB_FEATURE_ID;

/** USB class IDs */
typedef enum USB_CLASS_CODE {
	USB_CLASS_DEVICE			= 0x00,	//composite device: derive class from interfaces
	USB_CLASS_AUDIO 			= 0x01,
	USB_CLASS_CDC				= 0x02,
	USB_CLASS_HID				= 0x03,
	USB_CLASS_PTP				= 0x06,
	USB_CLASS_MASSTORAGE		= 0x08,
	USB_CLASS_VENDORSPECIFIC	= 0xff
} USB_CLASS_CODE;

/** USB endpoint types */
typedef enum USB_ENDPOINT_TYPE {
	USB_EPTYPE_CONTROL			= 0x00,
	USB_EPTYPE_ISOCHRONOUS		= 0x01,
	USB_EPTYPE_BULK				= 0x02,
	USB_EPTYPE_INTERRUPT		= 0x03
} USB_ENDPOINT_TYPE;

/** USB endpoint sync types */
typedef enum USB_ENDPOINT_SYNC {
	USB_EPSYNC_NONE				= 0x00,
	USB_EPSYNC_ASYNCHRONOUS		= 0x04,
	USB_EPSYNC_ADAPTIVE			= 0x08,
	USB_EPSYNC_SYNCHRONOUS		= 0x08
} USB_ENDPOINT_SYNC;

/** USB endpoint usage types */
typedef enum USB_ENDPOINT_USAGE {
	USB_EPUSAGE_DATA				= 0x00,
	USB_EPUSAGE_FEEDBACK			= 0x10,
	USB_EPUSAGE_EXPLICITFEEDBACK	= 0x20,
	USB_EPUSAGE_RESERVED			= 0x40
} USB_ENDPOINT_USAGE;

#pragma mark Descriptor convenience macros

/** splits a 16 bit unsigned integer into two bytes, little-endian - useful for various USB descriptors */
#define USB16(val) ((val) & 0xff),(((val) >> 8) & 0xff)

/** splits a 24 bit unsigned integer into three bytes, little-endian - useful for various USB descriptors */
#define USB24(val) ((val) & 0xff),(((val) >> 8) & 0xff),(((val) >> 16) & 0xff)

#endif