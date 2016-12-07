// Host layer Header file 
#pragma once 

/* Includes ------------------------------------------------------------------*/
#include "usb_regs.h"
#include "usb_core.h"

void  HCD_Init                 (USB_OTG_CORE_HANDLE *pdev ,USB_OTG_CORE_ID_TypeDef coreID);
uint32_t  HCD_HC_Init              (USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num); 
uint32_t  HCD_SubmitRequest        (USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num) ;

inline uint32_t HCD_GetCurrentSpeed (USB_OTG_CORE_HANDLE *pdev)
{    
    USB_OTG_HPRT0_TypeDef  HPRT0;
    HPRT0.d32 = USB_OTG_READ_REG32(pdev->regs.HPRT0);
    return HPRT0.b.prtspd;
}

inline uint32_t HCD_IsDeviceConnected(USB_OTG_CORE_HANDLE *pdev) {  return (pdev->host.ConnSts); }
// This function returns the frame number for sof packet
inline uint32_t HCD_GetCurrentFrame (USB_OTG_CORE_HANDLE *pdev) { return (USB_OTG_READ_REG32(&pdev->regs.HREGS->HFNUM) & 0xFFFF) ; }

inline void HCD_ResetPort(USB_OTG_CORE_HANDLE *pdev)
{
  /*
  Before starting to drive a USB reset, the application waits for the OTG 
  interrupt triggered by the debounce done bit (DBCDNE bit in OTG_FS_GOTGINT), 
  which indicates that the bus is stable again after the electrical debounce 
  caused by the attachment of a pull-up resistor on DP (FS) or DM (LS).
  */
  USB_OTG_ResetPort(pdev); 
}

// This function returns the last URBstate
inline URB_STATE HCD_GetURB_State (USB_OTG_CORE_HANDLE *pdev , uint8_t ch_num)  { return pdev->host.URB_State[ch_num] ; }
inline uint32_t HCD_GetXferCnt (USB_OTG_CORE_HANDLE *pdev, uint8_t ch_num) { return pdev->host.XferCnt[ch_num] ; }
inline HC_STATUS HCD_GetHCState (USB_OTG_CORE_HANDLE *pdev ,  uint8_t ch_num) { return pdev->host.HC_Status[ch_num]; }

