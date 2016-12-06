/**
  ******************************************************************************
  * @file    usb_hcd.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   Host Interface Layer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usb_core.h"
#include "usb_hcd.h"
#include "usb_conf.h"
#include "usb_bsp.h"

//  Initialize the HOST portion of the driver.
void HCD_Init(USB_OTG_CORE_HANDLE *pdev, USB_OTG_CORE_ID_TypeDef coreID)
{
  uint8_t i = 0;
  pdev->host.ConnSts = 0;
  
  for (i= 0; i< USB_OTG_MAX_TX_FIFOS; i++) {
      pdev->host.ErrCnt[i]  = 0;
      pdev->host.XferCnt[i] = 0;
      pdev->host.HC_Status[i] = HC_IDLE;
  }
  pdev->host.hc[0].max_packet  = 8; 

  USB_OTG_SelectCore(pdev, coreID);

  // --- USB OTG Disable GlobalInt
  USB_OTG_GAHBCFG_TypeDef ahbcfg;

  ahbcfg.d32 = 0;
  ahbcfg.b.glblintrmsk = 1; /* Enable interrupts */
  USB_OTG_MODIFY_REG32(&pdev->regs.GREGS->GAHBCFG, ahbcfg.d32, 0);

  // ----
  USB_OTG_CoreInit(pdev);

  // -- Setup mode : Force Host Mode 
  USB_OTG_GUSBCFG_TypeDef  usbcfg;
  usbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GUSBCFG);
  usbcfg.b.force_host = 1;
  USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GUSBCFG, usbcfg.d32);

  // -- delay 
  USB_OTG_BSP_mDelay(50);

  // -- init host
  USB_OTG_CoreInitHost(pdev);

  // Enable global int 
  USB_OTG_MODIFY_REG32(&pdev->regs.GREGS->GAHBCFG, 0, ahbcfg.d32);

}






/**
  * @brief  HCD_HC_Init 
  *         This function prepare a HC and start a transfer
  * @param  pdev: Selected device
  * @param  hc_num: Channel number 
  * @retval status 
  */
uint32_t HCD_HC_Init (USB_OTG_CORE_HANDLE *pdev , uint8_t hc_num) 
{
  return USB_OTG_HC_Init(pdev, hc_num);  
}

/**
  * @brief  HCD_SubmitRequest 
  *         This function prepare a HC and start a transfer
  * @param  pdev: Selected device
  * @param  hc_num: Channel number 
  * @retval status
  */
uint32_t HCD_SubmitRequest (USB_OTG_CORE_HANDLE *pdev , uint8_t hc_num) 
{
  
  pdev->host.URB_State[hc_num] =   URB_IDLE;  
  pdev->host.hc[hc_num].xfer_count = 0 ;
  return USB_OTG_HC_StartXfer(pdev, hc_num);
}

