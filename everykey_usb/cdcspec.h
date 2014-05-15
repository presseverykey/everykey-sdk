/** Definitions taken from the USB CDC Spec, V1.1.
 These definitions should be largely platform-independent. */

#ifndef _USBCDCSPEC_
#define _USBCDCSPEC_

#include "usbspec.h"

/** class specific interface classes - use in interface bClass fields */
typedef enum USB_CDC_INTERFACE_CLASS {
	USB_CDC_INTERFACE_COMMUNICATION_INTERFACE			= 0x02,
	USB_CDC_INTERFACE_DATA_INTERFACE					= 0x0a 
} USB_CDC_INTERFACE_CLASS;
	
/** communication interface subclass - use in interface subclass field 
 *  USB CDC subclass ids, defined in usbcdc11.pdf Table 16 p.28 (p.39 pdf)*/
typedef enum USB_CDC_CI_SUBCLASS {
	USB_CDC_SUBCLASS_UNDEFINED                         = 0x00,
	USB_CDC_SUBCLASS_DIRECT_LINE_CONTROL_MODEL         = 0x01,
	USB_CDC_SUBCLASS_ABSTRACT_CONTROL_MODEL            = 0x02,
	USB_CDC_SUBCLASS_TELEPHONE_CONTROL_MODEL           = 0x03,
	USB_CDC_SUBCLASS_MULTICHANNEL_CONTROL_MODEL        = 0x04,
	USB_CDC_SUBCLASS_CAPI_CONTROL_MODEL                = 0x05,
	USB_CDC_SUBCLASS_ETHERNET_NETWORKING_CONTROL_MODEL = 0x06,
	USB_CDC_SUBCLASS_ATM_NETWORKING_CONTROL_MODEL      = 0x07
} USB_CDC_CI_SUBCLASS;

/** communication interface protocol - use in interface bProtocol field */
typedef enum USB_CDC_CI_PROTOCOL {
	USB_CDC_CI_PROTOCOL_NONE							= 0x00,
	USB_CDC_CI_PROTOCOL_V25TER							= 0x01,
	USB_CDC_CI_PROTOCOL_VENDOR							= 0xff
} USB_CDC_CI_PROTOCOL;

/** data interface protocol - use in interface bProtocol field */
typedef enum USB_CDC_DI_PROTOCOL {
	USB_CDC_DI_PROTOCOL_NONE							= 0x00,
	USB_CDC_DI_PROTOCOL_I430							= 0x30,
	USB_CDC_DI_PROTOCOL_ISO3309							= 0x31,
	USB_CDC_DI_PROTOCOL_TRANSPARENT						= 0x32,
	USB_CDC_DI_PROTOCOL_Q921M							= 0x50,
	USB_CDC_DI_PROTOCOL_Q921							= 0x51,
	USB_CDC_DI_PROTOCOL_Q921TM							= 0x52,
	USB_CDC_DI_PROTOCOL_V42BIS							= 0x90,
	USB_CDC_DI_PROTOCOL_Q931							= 0x91,
	USB_CDC_DI_PROTOCOL_V120							= 0x92,
	USB_CDC_DI_PROTOCOL_CAPI20							= 0x93,
	USB_CDC_DI_PROTOCOL_PROT_UNIT_FUNC_DESC				= 0xfe,
	USB_CDC_DI_PROTOCOL_VENDOR							= 0xff
} USB_CDC_DI_PROTOCOL;

/** descriptor types for CDC-specific descriptors */
typedef enum USB_CDC_DESCRIPTOR_TYPE {
	USB_CDC_HEADER_FUNC_DESC							= 0x00,
	USB_CDC_CALL_MGMT_FUNC_DESC							= 0x01,
	USB_CDC_ABSTRACT_CONTROL_MODEL_FUNC_DESC			= 0x02,
	USB_CDC_DIRECT_LINE_MGMT_FUNC_DESC					= 0x03,
	USB_CDC_TEL_RINGER_FUNC_DESC						= 0x04,
	USB_CDC_TEL_LINE_STATE_REPORT_CAP_FUNC_DESC			= 0x05,
	USB_CDC_UNION_FUNC_DESC								= 0x06,
	USB_CDC_TEL_OP_MODES_FUNC_DESC						= 0x07,
	USB_CDC_USB_TERM_FUNC_DESC							= 0x08
} USB_CDC_DESCRIPTOR_TYPE;

/** class-specific request types */
typedef enum USB_CDC_REQUEST_TYPE {
	USB_CDC_REQUEST_SEND_ENCAPSULATED_COMMAND           = 0x00,
	USB_CDC_REQUEST_GET_ENCAPSULATED_RESPONSE           = 0x01,
	USB_CDC_REQUEST_SET_COMM_FEATURE                    = 0x02,
	USB_CDC_REQUEST_GET_COMM_FEATURE                    = 0x03,
	USB_CDC_REQUEST_CLEAR_COMM_FEATURE                  = 0x04,
	USB_CDC_REQUEST_SET_AUX_LINE_STATE                  = 0x10,
	USB_CDC_REQUEST_SET_HOOK_STATE                      = 0x11,
	USB_CDC_REQUEST_PULSE_SETUP                         = 0x12,
	USB_CDC_REQUEST_SEND_PULSE                          = 0x13,
	USB_CDC_REQUEST_SET_PULSE_TIME                      = 0x14,
	USB_CDC_REQUEST_RING_AUX_JACK                       = 0x15,
	USB_CDC_REQUEST_SET_LINE_CODING                     = 0x20,
	USB_CDC_REQUEST_GET_LINE_CODING                     = 0x21,
	USB_CDC_REQUEST_SET_CONTROL_LINE_STATE              = 0x22,
	USB_CDC_REQUEST_SEND_BREAK                          = 0x23,
	USB_CDC_REQUEST_SET_RINGER_PARMS                    = 0x30,
	USB_CDC_REQUEST_GET_RINGER_PARMS                    = 0x31,
	USB_CDC_REQUEST_SET_OPERATION_PARMS                 = 0x32,
	USB_CDC_REQUEST_GET_OPERATION_PARMS                 = 0x33,
	USB_CDC_REQUEST_SET_LINE_PARMS                      = 0x34,
	USB_CDC_REQUEST_GET_LINE_PARMS                      = 0x35,
	USB_CDC_REQUEST_DIAL_DIGITS                         = 0x36,
	USB_CDC_REQUEST_SET_UNIT_PARAMETER                  = 0x37,
	USB_CDC_REQUEST_GET_UNIT_PARAMETER                  = 0x38,
	USB_CDC_REQUEST_CLEAR_UNIT_PARAMETER                = 0x39,
	USB_CDC_REQUEST_GET_PROFILE                         = 0x3A,
	USB_CDC_REQUEST_SET_ETHERNET_MULTICAST_FILTERS      = 0x40,
	USB_CDC_REQUEST_SET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER = 0x41,
	USB_CDC_REQUEST_GET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER = 0x42,
	USB_CDC_REQUEST_SET_ETHERNET_PACKET_FILTER          = 0x43,
	USB_CDC_REQUEST_GET_ETHERNET_STATISTIC              = 0x44,
	USB_CDC_REQUEST_SET_ATM_DATA_FORMAT                 = 0x50,
	USB_CDC_REQUEST_GET_ATM_DEVICE_STATISTICS           = 0x51,
	USB_CDC_REQUEST_SET_ATM_DEFAULT_VC                  = 0x52,
	USB_CDC_REQUEST_GET_ATM_VC_STATISTICS               = 0x53
} USB_CDC_REQUEST_TYPE;

/** notification types to use in the notification bRequest field */
typedef enum USB_CDC_NOTIFICATION_TYPE {
	USB_CDC_NOTIFICATION_NETWORK_CONNECTION				= 0x00,
	USB_CDC_NOTIFICATION_RESPONSE_AVAILABLE				= 0x01,
	USB_CDC_NOTIFICATION_AUX_JACK_HOOK_STATE			= 0x08,
	USB_CDC_NOTIFICATION_RING_DETECT					= 0x09,
	USB_CDC_NOTIFICATION_SERIAL_STATE					= 0x20,
	USB_CDC_NOTIFICATION_CALL_STATE_CHANGE				= 0x28,
	USB_CDC_NOTIFICATION_LINE_STATE_CHANGE				= 0x29,
	USB_CDC_NOTIFICATION_CONNECTION_SPEED_CHANGE		= 0x2a
} USB_CDC_NOTIFICATION_TYPE;

/** bits to set in the serialState field of USBCDC_SerialState_Notification_Struct */
typedef enum USB_CDC_SERIALSTATE_BITS {
	USB_CDC_SERIALSTATE_BUFFER_OVERRUN					= 1 << 6,
	USB_CDC_SERIALSTATE_PARITY_ERROR					= 1 << 5,
	USB_CDC_SERIALSTATE_FRAMING_ERROR					= 1 << 4,
	USB_CDC_SERIALSTATE_RING_DETECTION					= 1 << 3,
	USB_CDC_SERIALSTATE_BREAK_DETECTION					= 1 << 2,
	USB_CDC_SERIALSTATE_TX_CARRIER						= 1 << 1,
	USB_CDC_SERIALSTATE_RX_CARRIER						= 1 << 0,
} USB_CDC_SERIALSTATE_BITS;

/** data sent to the host via interrupt pipe */
typedef struct _USB_Setup_Packet USBCDC_Nodata_Notification_Struct;

/** data sent to the host via interrupt pipe for serialState notifications.
 2 bytes data payload */
typedef struct _USBCDC_SerialState_Notification_Struct {
	USB_Setup_Packet setupPacket;
	uint16_t serialState;
} __attribute__((packed)) USBCDC_SerialState_Notification_Struct;

/** Number of stop bits in the USB_CDC_LINECODING_STRUCT */
typedef enum USB_CDC_LINECODING_CHARFORMAT {
	USB_CDC_LINECODING_STOP_1                           = 0,
	USB_CDC_LINECODING_STOP_1_5                         = 1,
	USB_CDC_LINECODING_STOP_2                           = 2
} USB_CDC_LINECODING_CHARFORMAT;

/** Parity type in the USB_CDC_LINECODING_STRUCT */
typedef enum USB_CDC_LINECODING_PARITY {
	USB_CDC_PARITY_NONE                                 = 0,
	USB_CDC_PARITY_ODD                                  = 1,
	USB_CDC_PARITY_EVEN                                 = 2,
	USB_CDC_PARITY_MARK                                 = 3,
	USB_CDC_PARITY_SPACE                                = 4
} USB_CDC_LINECODING_PARITY;

/** line coding structure for GetLineCoding and SetLineCoding commands */
typedef struct USB_CDC_Linecoding_Struct {
	uint32_t dwDTERRate;                //Data rate in bps
	uint8_t bCharFormat;                //Number of Stop bits - USB_CDC_LINECODING_CHARFORMAT 
	uint8_t bParityType;                //Parity type - USB_CDC_LINECODING_PARITY
	uint8_t bDataBits;                  //number of data bits (5/6/7/8 or 16)
} __attribute__((packed)) USB_CDC_Linecoding_Struct;

/** feature codes for SetCommFeature/GetCommFeature/ClearCommFeature */
typedef enum USB_CDC_COMM_FEATURE_CODE {
	USB_CDC_COMM_FEATURE_ABSTRACT_STATE                 = 1,
	USB_CDC_COMM_FEATURE_COUNTRY_SETTING                = 2
} USB_CDC_COMM_FEATURE_CODE;

/** bits for the abstract state comm feature */
typedef enum USB_CDC_COMM_FEATURE_ABSTRACT_STATE_BITS {
	USB_CDC_COMM_FEATURE_ABSTRACT_STATE_MULTIPLEX		= 1 << 1,
	USB_CDC_COMM_FEATURE_ABSTRACT_STATE_IDLE			= 1 << 0
} USB_CDC_COMM_FEATURE_ABSTRACT_STATE_BITS;

#endif
