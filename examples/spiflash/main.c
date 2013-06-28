#include "anykey/anykey.h"

const uint8_t deviceDescriptor[] = {
	0x12,							//bLength: length of this structure in bytes (18)
	USB_DESC_DEVICE,				//bDescriptorType: usb device descriptor
	0x00, 0x02,						//bcdUSB: 0200 (Little Endian) - USB 2.0 compliant
	0x00,							//bDeviceClass: Device class (0 = interfaces specify class)
	0x00,							//bDeviceSubClass: Device subclass (must be 0 if bDeviceClass is 0)
	0x00,							//bDeviceProtocol: 0 for no specific device-level protocols
	USB_MAX_COMMAND_PACKET_SIZE,	//bMaxPacketSize0: Max packet size for control endpoint
	0x34, 0x12,                     //idVendor: 16 bit vendor id
	0x78, 0x56,                     //idProduct: 16 bit product id
	0x00, 0x01,						//bcdDevice: Device release version
	0x01,                           //iManufacturer: Manufacturer string index
	0x02,                           //iProduct: Product string index
	0x03,                           //iSerialNumber: Serial number string index
	0x01                            //bNumConfigurations: Number of configurations
};

const uint8_t languages[] = {
	0x04,							//bLength: length of this descriptor in bytes (4)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	0x09,0x04						//wLangID[]: An array of 16 bit language codes (LE). 0x0409: English (US)
};

const uint8_t manufacturerName[] = {
	0x22,							//bLength: length of this descriptor in bytes (34)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	'P',0,'r',0,'e',0,'s',0,'s',0,' ',0,'A',0,'n',0,'y',0,' ',0,'K',0,'e',0,'y',0,' ',0,'U',0,'G',0	//bString[]: String (UTF16LE, not terminated)
};

const uint8_t deviceName[] = {
	0x12,							//bLength: length of this descriptor in bytes (18)
	USB_DESC_STRING,				//bDescriptorType: string descriptor
	'A',0,'n',0,'y',0,'k',0,'e',0,'y',0,'0',0,'x',0			//bString[]: String (UTF16LE, not terminated)
};

const uint8_t configDescriptor[] = {
	0x09,							//bLength: length of this descriptor in bytes (9)
	USB_DESC_CONFIGURATION,			//bDescriptorType: configuration descriptor
	0x12, 0x00,						//wTotalLen: Total length, including attached interface and endpoint descriptors
	0x01,							//bNumInterfaces: Number of interfaces (1)
	0x01,							//bConfigurationValue: Number to set to activate this config
	0x00,							//iConfiguration: configuration string index (0 = not available)
	0x80,							//bmAttributes: Not self-powered, no remote wakeup
	0x32,							//bMaxPower: Max power in 2mA steps (0x32 = 50 = 100mA)
	
	//interface 0
	0x09,							//bLength: length of this descriptor in bytes (9)
	USB_DESC_INTERFACE,				//bDescriptor type: constant indicating that this is an interface descriptor
	0x00,							//bInterfaceNumber: Interface index, 0-based
	0x00,							//bAlternateSetting
	0x00,							//bNumEndpoints: Number of endpoints excluding control endpoint
	0xff,							//bInterfaceClass: interface class (0xff = vendor specific)
	0x00,							//bInterfaceSubClass
	0xff,							//bInterfaceProtocol (0xff = vendor specific)
	0x00							//iInterface: String index (0x00 = not available)
	
	//add endpoint descriptors here...
};
	
USB_Device_Struct myUSBDevice;

#define FLASHSEL_PORT 2
#define FLASHSEL_PIN 4

#define SPIFLASH_CMD_GETSTATUS 0x05
#define SPIFLASH_CMD_GETJEDECID 0x9f
#define SPIFLASH_STATUS_BUSY 0x01
#define MAX_BUSY_RETRIES 100000000


void delay (uint32_t duration) {
	volatile uint32_t wait;
	for (wait = 0; wait < duration; wait++) { NOP }
}

void SPIFLASH_Init() {
	GPIO_SetDir(FLASHSEL_PORT, FLASHSEL_PIN, GPIO_Output);
	GPIO_WriteOutput(FLASHSEL_PORT, FLASHSEL_PIN, true);	//deselect chip (low active)	
	SSP_Init(4, 8, SSP_CR0_FRF_SPI, true, true, true);
}

bool SPIFLASH_WaitReady() {
	uint32_t retries;
	bool busy;
	for (retries = 0; retries < MAX_BUSY_RETRIES; retries++) {
		GPIO_WriteOutput(FLASHSEL_PORT, FLASHSEL_PIN, false);	// start talking to chip
		SSP_Transfer(SPIFLASH_CMD_GETSTATUS);
		uint16_t status = SSP_Transfer(0x00);
		GPIO_WriteOutput(FLASHSEL_PORT, FLASHSEL_PIN, true);	// stop talking to chip
		busy = (status & SPIFLASH_STATUS_BUSY);
		if (!busy) break;
		delay(100000);
	}
	return !busy;
}

uint32_t SPIFLASH_ReadDeviceId() {
	GPIO_WriteOutput(FLASHSEL_PORT, FLASHSEL_PIN, false);	// start talking to chip
	SSP_Transfer(SPIFLASH_CMD_GETJEDECID);					// write command JEDEC Device ID
	uint8_t manufacturerId = SSP_Transfer(0x00);			// Read return
	uint8_t memoryType = SSP_Transfer(0x00);				// Read return
	uint8_t capacity = SSP_Transfer(0x00);					// Read return
	GPIO_WriteOutput(FLASHSEL_PORT, FLASHSEL_PIN, true);	// stop talking to chip
	return (manufacturerId << 16) | (memoryType << 8) | capacity;
}

uint8_t	NibbleToHexChar(uint8_t value) {
	if (value < 10) return '0'+value;
	else if (value < 16) return 'a'+value-10;
	else return '?';
}

uint8_t versionString[14];

#define LED_PORT 0
#define LED_PIN 7

void main(void) {
	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
	GPIO_WriteOutput(LED_PORT, LED_PIN, true);
	
	SPIFLASH_Init();
	SPIFLASH_WaitReady();

	GPIO_WriteOutput(LED_PORT, LED_PIN, false);

//	while (true) {
//		uint32_t deviceId = SPIFLASH_ReadDeviceId();
//		delay(1000000);
//	}
	
	uint32_t deviceId = SPIFLASH_ReadDeviceId();
	
	versionString[ 0] = 14;
	versionString[ 1] = USB_DESC_STRING;
	versionString[ 2] = NibbleToHexChar((deviceId >> 20) & 0x0f);
	versionString[ 3] = 0;
	versionString[ 4] = NibbleToHexChar((deviceId >> 16) & 0x0f);
	versionString[ 5] = 0;
	versionString[ 6] = NibbleToHexChar((deviceId >> 12) & 0x0f);
	versionString[ 7] = 0;
	versionString[ 8] = NibbleToHexChar((deviceId >> 8) & 0x0f);
	versionString[ 9] = 0;
	versionString[10] = NibbleToHexChar((deviceId >> 4) & 0x0f);
	versionString[11] = 0;
	versionString[12] = NibbleToHexChar(deviceId & 0x0f);
	versionString[13] = 0;
	
	myUSBDevice.deviceDescriptor = deviceDescriptor;
	myUSBDevice.configurationCount = 1;
	myUSBDevice.configurationDescriptors[0] = configDescriptor;
	myUSBDevice.stringCount = 4;
	myUSBDevice.strings[0] = languages;
	myUSBDevice.strings[1] = manufacturerName;
	myUSBDevice.strings[2] = deviceName;
	myUSBDevice.strings[3] = versionString;
	myUSBDevice.extendedControlSetupCallback = NULL;
	myUSBDevice.endpointDataCallback = NULL;
	
	USB_Init(&myUSBDevice);
	USB_SoftConnect();
}
