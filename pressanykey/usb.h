#ifndef _USB_
#define _USB_

#include "types.h"

typedef void USB_VoidEventHandler(void);
typedef USB_VoidEventHandler* USB_VoidEventHandlerPtr;

typedef void USB_UInt32EventHandler(void);
typedef USB_UInt32EventHandler* USB_UInt32EventHandlerPtr;

typedef struct USB_DEVICE_STRUCT {
	USB_VoidEventHandlerPtr powerUpHandler;
	USB_VoidEventHandlerPtr powerDownHandler;
	USB_VoidEventHandlerPtr suspendHandler;
	USB_VoidEventHandlerPtr resumeHandler;
	USB_VoidEventHandlerPtr wakeupHandler;
	USB_VoidEventHandlerPtr sofHandler;
	USB_UInt32EventHandlerPtr errorHandler;
	USB_UInt32EventHandlerPtr epHandler[5];
	USB_UInt32EventHandlerPtr configureHandler;
	USB_UInt32EventHandlerPtr interfaceHandler;
	USB_UInt32EventHandlerPtr featureHandler;
} USB_DEVICE_STRUCT;

/** powers up required blocks, sets up clock etc. Leaves soft-connect disconnected. */
void USB_Init(USB_DEVICE_STRUCT* );

/** enable connection from client side (soft-connect) */
void USB_SoftConnect(void);

/** disable connection from client side (soft-connect) */
void USB_SoftDisconnect(void);

/** returns whether the device is connected (VBUS present) */
bool USB_Connected(void);





#endif