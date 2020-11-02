/*******************************************************************************
 * @file descriptors.c
 * @brief USB descriptors.
 *******************************************************************************/

//=============================================================================
// src/descriptors.c: generated by Hardware Configurator
//
// This file is only generated if it does not exist. Modifications in this file
// will persist even if Configurator generates code. To refresh this file,
// you must first delete it and then regenerate code.
//=============================================================================
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <si_toolchain.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include <efm8_usb.h>
#include "descriptors.h"

#ifdef __cplusplus
extern "C" {
#endif

// Device Descriptor
SI_SEGMENT_VARIABLE(deviceDesc[],
		const USB_DeviceDescriptor_TypeDef,
		SI_SEG_CODE) =
{
	USB_DEVICE_DESCSIZE,             // bLength
	USB_DEVICE_DESCRIPTOR,// bLength
	htole16(0x0200),// bcdUSB
	0,// bDeviceClass
	0,// bDeviceSubClass
	0,// bDeviceProtocol
	64,// bMaxPacketSize
	USB_VENDOR_ID,// idVendor
	USB_PRODUCT_ID,// idProduct
	htole16(0x0100),// bcdDevice
	1,// iManufacturer
	2,// iProduct
	3,// iSerialNumber
	1,// bNumConfigurations
};


// Configuration Descriptor
SI_SEGMENT_VARIABLE(configDesc[],
		const uint8_t,
		SI_SEG_CODE) =
{
		  USB_CONFIG_DESCSIZE,             // bLength
		  USB_CONFIG_DESCRIPTOR,           // bDescriptorType
		  0x29,                            // wTotalLength(LSB)
		  0x00,                            // wTotalLength(MSB)
		  1,                               // bNumInterfaces
		  1,                               // bConfigurationValue
		  0,                               // iConfiguration
		  CONFIG_DESC_BM_RESERVED_D7,      // bmAttrib: Bus powered
		  CONFIG_DESC_MAXPOWER_mA(100),    // bMaxPower: 100 mA

		  //Interface 0 Descriptor
		  USB_INTERFACE_DESCSIZE,          // bLength
		  USB_INTERFACE_DESCRIPTOR,        // bDescriptorType
		  0,                               // bInterfaceNumber
		  0,                               // bAlternateSetting
		  2,                               // bNumEndpoints
		  USB_CLASS_HID,                   // bInterfaceClass: HID (Human Interface Device)
		  0,                               // bInterfaceSubClass
		  0,                               // bInterfaceProtocol
		  0,                               // iInterface

		  // Class Descriptor (HID Descriptor)
		  USB_HID_DESCSIZE,                // bLength
		  USB_HID_DESCRIPTOR,              // bLength
		  0x11,                            // bcdHID (LSB)
		  0x01,                            // bcdHID (MSB)
		  0,                               // bCountryCode
		  1,                               // bNumDescriptors
		  USB_HID_REPORT_DESCRIPTOR,       // bDescriptorType
		  sizeof( ReportDescriptor0 ),     // wDescriptorLength(LSB)
		  sizeof( ReportDescriptor0 )>>8,    // wDescriptorLength(MSB)

		  // Endpoint 1 IN Descriptor
		  USB_ENDPOINT_DESCSIZE,           // bLength
		  USB_ENDPOINT_DESCRIPTOR,         // bDescriptorType
		  0x81,                            // bEndpointAddress
		  USB_EPTYPE_INTR,                 // bAttrib
		  0x40,                            // wMaxPacketSize (LSB)
		  0x00,                            // wMaxPacketSize (MSB)
		  1,                               // bInterval

		  // Endpoint 1 OUT Descriptor
		  USB_ENDPOINT_DESCSIZE,           // bLength
		  USB_ENDPOINT_DESCRIPTOR,         // bDescriptorType
		  0x01,                            // bEndpointAddress
		  USB_EPTYPE_INTR,                 // bAttrib
		  0x40,                            // wMaxPacketSize (LSB)
		  0x00,                            // wMaxPacketSize (MSB)
		  1,                               // bInterval
};

// report 1: set all meters, 4 bytes, four levels
// report 2: set one meter, 2 bytes, meter #, level
// report 3: set lamps, 3 bytes, lamp #, pwm level

// HID Report Descriptor for Interface 0
SI_SEGMENT_VARIABLE(ReportDescriptor0[53],
		const uint8_t,
		SI_SEG_CODE) =
{

		0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
		0x09, 0x01,        // Usage (0x01)
		0xA1, 0x01,        // Collection (Application)

		0x85, 0x01,        //   Report ID (1)
		0x95, 0x04,        //   Report Count (4)
		0x75, 0x08,        //   Report Size (8)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x15, 0x00,        //   Logical Minimum (0)
		0x09, 0x01,        //   Usage (0x01)
		0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

		0x85, 0x02,        //   Report ID (2)
		0x95, 0x02,        //   Report Count (2)
		0x75, 0x08,        //   Report Size (8)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x15, 0x00,        //   Logical Minimum (0)
		0x09, 0x01,        //   Usage (0x01)
		0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

		0x85, 0x03,        //   Report ID (3)
		0x95, 0x03,        //   Report Count (3)
		0x75, 0x08,        //   Report Size (8)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x15, 0x00,        //   Logical Minimum (0)
		0x09, 0x01,        //   Usage (0x01)
		0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

		0xC0,              // End Collection

		// 53 bytes
};

//                      1   2   3   4   5   6   7   8   9  10   1   2   3   4   5   6   7   8   9  20   1   2   3   4   5   6   7   8   9  30   1   2   3   4   5
#define LANG_STRING   htole16( SLAB_USB_LANGUAGE )
#define MFR_STRING    'b','i','k','e','r','g','l','e','n','.','c','o','m','\0'
#define MFR_SIZE      14
#define PROD_STRING   'A','n','a','l','o','g',' ','P','a','n','e','l',' ','M','e','t','e','r','s','\0'
#define PROD_SIZE     20
#define SER_STRING    '0','0','0','0','-','0','0','0','0','-','0','0','0','1','\0'
#define SER_SIZE      15
#define INT0_STRING   'M','e','t','e','r','s',' ','a','n','d',' ','L','i','g','h','t','s','\0'
#define INT0_SIZE     18

LANGID_STATIC_CONST_STRING_DESC( langDesc[], LANG_STRING );
UTF16LE_PACKED_STATIC_CONST_STRING_DESC( mfrDesc[], MFR_STRING, MFR_SIZE);
UTF16LE_PACKED_STATIC_CONST_STRING_DESC( prodDesc[], PROD_STRING, PROD_SIZE);
UTF16LE_PACKED_STATIC_CONST_STRING_DESC( serDesc[], SER_STRING, SER_SIZE);
UTF16LE_PACKED_STATIC_CONST_STRING_DESC( int0Desc[], INT0_STRING, INT0_SIZE);

//-----------------------------------------------------------------------------
SI_SEGMENT_VARIABLE_SEGMENT_POINTER(myUsbStringTable_USEnglish[]
		, static const USB_StringDescriptor_TypeDef,
		SI_SEG_GENERIC,
		const SI_SEG_CODE) =
{
	(SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_CODE))langDesc,
	mfrDesc,
	prodDesc,
	serDesc,
	int0Desc
};

//-----------------------------------------------------------------------------
SI_SEGMENT_VARIABLE(initstruct,
		const USBD_Init_TypeDef,
		SI_SEG_CODE) =
{
	(SI_VARIABLE_SEGMENT_POINTER(, USB_DeviceDescriptor_TypeDef, SI_SEG_GENERIC))deviceDesc, // deviceDescriptor
	(SI_VARIABLE_SEGMENT_POINTER(, USB_ConfigurationDescriptor_TypeDef, SI_SEG_GENERIC))configDesc,// configDescriptor
	(SI_VARIABLE_SEGMENT_POINTER(, USB_StringTable_TypeDef, SI_SEG_GENERIC))myUsbStringTable_USEnglish,// stringDescriptors
	sizeof(myUsbStringTable_USEnglish) / sizeof(myUsbStringTable_USEnglish[0])// numberOfStrings
};

#ifdef __cplusplus
}
#endif

