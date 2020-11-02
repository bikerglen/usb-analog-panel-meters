#ifndef __SILICON_LABS_DESCRIPTORS_H
#define __SILICON_LABS_DESCRIPTORS_H

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include "si_toolchain.h"
#include "efm8_usb.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USB_VENDOR_ID  htole16(0x4247)
#define USB_PRODUCT_ID htole16(0x000A)

#define HID_VENDOR_IFC 0

extern SI_SEGMENT_VARIABLE(ReportDescriptor0[53], const uint8_t, SI_SEG_CODE);
extern SI_SEGMENT_VARIABLE(deviceDesc[], const USB_DeviceDescriptor_TypeDef, SI_SEG_CODE);
extern SI_SEGMENT_VARIABLE(configDesc[], const uint8_t, SI_SEG_CODE);
extern SI_SEGMENT_VARIABLE(initstruct, const USBD_Init_TypeDef, SI_SEG_CODE);

#ifdef __cplusplus
}
#endif

#endif // __SILICON_LABS_DESCRIPTORS_H

