//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SI_EFM8UB1_Register_Enums.h>
#include <efm8_usb.h>
#include "descriptors.h"
#include "idle.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Port sbits
//-----------------------------------------------------------------------------

SI_SBIT(SW1,       SFR_P0, 4);

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

uint8_t tmpBuffer;
uint8_t xdata txBuffer[64];
uint8_t xdata rxBuffer[64];

uint8_t volatile irqMetersUpdateFlags = 0;
uint8_t volatile irqMetersLevels[4];


//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void USBD_EnterHandler (void)
{
	// nothing
}


void USBD_ExitHandler(void)
{
	// nothing
}


void USBD_ResetCb (void)
{
	// nothing
}


void USBD_SofCb (uint16_t sofNr)
{
	UNREFERENCED_ARGUMENT(sofNr);

	idleTimerTick();

	// Check if the device should send a report
	if (isIdleTimerExpired() == true) {
	    if (USBD_EpIsBusy(EP1IN) == false) {
	    }
	}
}

void USBD_DeviceStateChangeCb (USBD_State_TypeDef oldState,
							   USBD_State_TypeDef newState)
{
	// not configured or in suspend
	if (newState < USBD_STATE_SUSPENDED)
	{
	}
	// Entering suspend mode, power internal and external blocks down
	else if (newState == USBD_STATE_SUSPENDED)
	{
		// Abort any pending transfer
		USBD_AbortTransfer (EP1IN);
	}
	else if (newState == USBD_STATE_CONFIGURED)
	{
		idleTimerSet(POLL_RATE);
		USBD_Read (EP1OUT, rxBuffer, 64, true);
	}

	// Exiting suspend mode, power internal and external blocks up
	if (oldState == USBD_STATE_SUSPENDED)
	{
	}
}


bool USBD_IsSelfPoweredCb (void)
{
	return true;
}


USB_Status_TypeDef USBD_SetupCmdCb(SI_VARIABLE_SEGMENT_POINTER(
                                     setup,
                                     USB_Setup_TypeDef,
                                     MEM_MODEL_SEG))
{
  USB_Status_TypeDef retVal = USB_STATUS_REQ_UNHANDLED;

  if ((setup->bmRequestType.Type == USB_SETUP_TYPE_STANDARD)
      && (setup->bmRequestType.Direction == USB_SETUP_DIR_IN)
      && (setup->bmRequestType.Recipient == USB_SETUP_RECIPIENT_INTERFACE))
  {
    // A HID device must extend the standard GET_DESCRIPTOR command
    // with support for HID descriptors.
    switch (setup->bRequest)
    {
      case GET_DESCRIPTOR:
        if ((setup->wValue >> 8) == USB_HID_REPORT_DESCRIPTOR)
        {
          switch (setup->wIndex)
          {
            case 0: // Interface 0

              USBD_Write(EP0,
                         (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))ReportDescriptor0,
                         EFM8_MIN(sizeof(ReportDescriptor0), setup->wLength),
                         false);
              retVal = USB_STATUS_OK;
              break;

            default: // Unhandled Interface
              break;
          }
        }
        else if ((setup->wValue >> 8) == USB_HID_DESCRIPTOR)
        {
          switch (setup->wIndex)
          {
            case 0: // Interface 0

              USBD_Write(EP0,
                         (SI_VARIABLE_SEGMENT_POINTER(, uint8_t, SI_SEG_GENERIC))(&configDesc[18]),
                         EFM8_MIN(USB_HID_DESCSIZE, setup->wLength),
                         false);
              retVal = USB_STATUS_OK;
              break;

            default: // Unhandled Interface
              break;
          }
        }
        break;
    }
  }
  else if ((setup->bmRequestType.Type == USB_SETUP_TYPE_CLASS)
           && (setup->bmRequestType.Recipient == USB_SETUP_RECIPIENT_INTERFACE)
           && (setup->wIndex == HID_VENDOR_IFC))
  {
    // Implement the necessary HID class specific commands.
    switch (setup->bRequest)
    {
      case USB_HID_SET_IDLE:
        if (((setup->wValue & 0xFF) == 0)             // Report ID
            && (setup->wLength == 0)
            && (setup->bmRequestType.Direction != USB_SETUP_DIR_IN))
        {
          idleTimerSet(setup->wValue >> 8);
          retVal = USB_STATUS_OK;
        }
        break;

      case USB_HID_GET_IDLE:
        if ((setup->wValue == 0)                      // Report ID
            && (setup->wLength == 1)
            && (setup->bmRequestType.Direction == USB_SETUP_DIR_IN))
        {
          tmpBuffer = idleGetRate();
          USBD_Write(EP0, &tmpBuffer, 1, false);
          retVal = USB_STATUS_OK;
        }
        break;
    }
  }

  return retVal;
}


// Technicaly SPI is fast enough I could do the updates at interrupt time
// without issues but I'm not sure I never want to be able to use the
// SPI in the main loop so I'm going to copy the data out of the USB
// rx packet and set a flag that lets the mainline code know it needs
// update the meters.

uint16_t USBD_XferCompleteCb(uint8_t epAddr, USB_Status_TypeDef status,
		uint16_t xferred, uint16_t remaining)
{
	uint8_t led;

	UNREFERENCED_ARGUMENT(status);
    UNREFERENCED_ARGUMENT(xferred);
	UNREFERENCED_ARGUMENT(remaining);

	if (epAddr == EP1OUT) {
		switch (rxBuffer[0]) {
			case 0x01:
				irqMetersUpdateFlags = 0x0f;
				irqMetersLevels[0] = rxBuffer[1];
				irqMetersLevels[1] = rxBuffer[2];
				irqMetersLevels[2] = rxBuffer[3];
				irqMetersLevels[3] = rxBuffer[4];
				break;
			case 0x02:
				if ((rxBuffer[1] >= 0) && (rxBuffer[1] <= 3)) {
					irqMetersUpdateFlags = 1 << rxBuffer[1];
					irqMetersLevels[rxBuffer[1]] = rxBuffer[2];
				}
				break;
			case 0x03:
				break;
		}
		USBD_Read (EP1OUT, rxBuffer, 64, true);
	}

	return 0;
}
