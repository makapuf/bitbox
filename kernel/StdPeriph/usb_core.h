/**
  ******************************************************************************
  * @file    usb_core.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   Header of the Core Layer
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CORE_H__
#define __USB_CORE_H__

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usb_regs.h"
#include "usb_defines.h"


/** @addtogroup USB_OTG_DRIVER
  * @{
  */
  
/** @defgroup USB_CORE
  * @brief usb otg driver core layer
  * @{
  */ 


/** @defgroup USB_CORE_Exported_Defines
  * @{
  */ 

#define USB_OTG_EP0_IDLE                          0
#define USB_OTG_EP0_SETUP                         1
#define USB_OTG_EP0_DATA_IN                       2
#define USB_OTG_EP0_DATA_OUT                      3
#define USB_OTG_EP0_STATUS_IN                     4
#define USB_OTG_EP0_STATUS_OUT                    5
#define USB_OTG_EP0_STALL                         6

#define USB_OTG_EP_TX_DIS       0x0000
#define USB_OTG_EP_TX_STALL     0x0010
#define USB_OTG_EP_TX_NAK       0x0020
#define USB_OTG_EP_TX_VALID     0x0030
 
#define USB_OTG_EP_RX_DIS       0x0000
#define USB_OTG_EP_RX_STALL     0x1000
#define USB_OTG_EP_RX_NAK       0x2000
#define USB_OTG_EP_RX_VALID     0x3000
/**
  * @}
  */ 
#define   MAX_DATA_LENGTH                        0x200

/** @defgroup USB_CORE_Exported_Types
  * @{
  */ 


typedef enum {
  USB_OTG_OK = 0,
  USB_OTG_FAIL
}USB_OTG_STS;

typedef enum {
  HC_IDLE = 0,
  HC_XFRC,
  HC_HALTED,
  HC_NAK,
  HC_NYET,
  HC_STALL,
  HC_XACTERR,  
  HC_BBLERR,   
  HC_DATATGLERR,  
}HC_STATUS;

typedef enum {
  URB_IDLE = 0,
  URB_DONE,
  URB_NOTREADY,
  URB_ERROR,
  URB_STALL
}URB_STATE;

typedef enum {
  CTRL_START = 0,
  CTRL_XFRC,
  CTRL_HALTED,
  CTRL_NAK,
  CTRL_STALL,
  CTRL_XACTERR,  
  CTRL_BBLERR,   
  CTRL_DATATGLERR,  
  CTRL_FAIL
}CTRL_STATUS;


typedef struct USB_OTG_hc
{
  uint8_t       dev_addr ;
  uint8_t       ep_num;
  uint8_t       ep_is_in;
  uint8_t       speed;
  uint8_t       do_ping;  
  uint8_t       ep_type;
  uint16_t      max_packet;
  uint8_t       data_pid;
  uint8_t       *xfer_buff;
  uint32_t      xfer_len;
  uint32_t      xfer_count;  
  uint8_t       toggle_in;
  uint8_t       toggle_out;
  uint32_t       dma_addr;  
}
USB_OTG_HC , *PUSB_OTG_HC;

typedef struct USB_OTG_ep
{
  uint8_t        num;
  uint8_t        is_in;
  uint8_t        is_stall;  
  uint8_t        type;
  uint8_t        data_pid_start;
  uint8_t        even_odd_frame;
  uint16_t       tx_fifo_num;
  uint32_t       maxpacket;
  /* transaction level variables*/
  uint8_t        *xfer_buff;
  uint32_t       dma_addr;  
  uint32_t       xfer_len;
  uint32_t       xfer_count;
  /* Transfer level variables*/  
  uint32_t       rem_data_len;
  uint32_t       total_data_len;
  uint32_t       ctl_data_len;  

}

USB_OTG_EP , *PUSB_OTG_EP;



typedef struct USB_OTG_core_cfg
{
  uint8_t       host_channels;
  uint8_t       dev_endpoints;
  uint8_t       speed;
  uint8_t       dma_enable;
  uint16_t      mps;
  uint16_t      TotalFifoSize;
  uint8_t       phy_itface;
  uint8_t       Sof_output;
  uint8_t       low_power;
  uint8_t       coreID;
 
}
USB_OTG_CORE_CFGS, *PUSB_OTG_CORE_CFGS;



typedef  struct  usb_setup_req {
    
    uint8_t   bmRequest;                      
    uint8_t   bRequest;                           
    uint16_t  wValue;                             
    uint16_t  wIndex;                             
    uint16_t  wLength;                            
} USB_SETUP_REQ;

typedef struct _HCD
{
  uint8_t                  Rx_Buffer [MAX_DATA_LENGTH];  
  __IO uint32_t            ConnSts;
  __IO uint32_t            ErrCnt[USB_OTG_MAX_TX_FIFOS];
  __IO uint32_t            XferCnt[USB_OTG_MAX_TX_FIFOS];
  __IO HC_STATUS           HC_Status[USB_OTG_MAX_TX_FIFOS];  
  __IO URB_STATE           URB_State[USB_OTG_MAX_TX_FIFOS];
  USB_OTG_HC               hc [USB_OTG_MAX_TX_FIFOS];
  uint16_t                 channel [USB_OTG_MAX_TX_FIFOS];
//  USB_OTG_hPort_TypeDef    *port_cb;  
}
HCD_DEV , *USB_OTG_USBH_PDEV;


typedef struct _OTG
{
  uint8_t    OTG_State;
  uint8_t    OTG_PrevState;  
  uint8_t    OTG_Mode;    
}
OTG_DEV , *USB_OTG_USBO_PDEV;

typedef struct USB_OTG_handle
{
  USB_OTG_CORE_CFGS    cfg;
  USB_OTG_CORE_REGS    regs;
  HCD_DEV     host;
}
USB_OTG_CORE_HANDLE , *PUSB_OTG_CORE_HANDLE;



USB_OTG_STS  USB_OTG_CoreInit        (USB_OTG_CORE_HANDLE *pdev);
USB_OTG_STS  USB_OTG_SelectCore      (USB_OTG_CORE_HANDLE *pdev, USB_OTG_CORE_ID_TypeDef coreID);
USB_OTG_STS  USB_OTG_EnableGlobalInt (USB_OTG_CORE_HANDLE *pdev);
USB_OTG_STS  USB_OTG_DisableGlobalInt(USB_OTG_CORE_HANDLE *pdev);
void*        USB_OTG_ReadPacket   (USB_OTG_CORE_HANDLE *pdev ,    uint8_t *dest,    uint16_t len);
USB_OTG_STS  USB_OTG_WritePacket     (USB_OTG_CORE_HANDLE *pdev ,    uint8_t *src,    uint8_t ch_ep_num,    uint16_t len);
USB_OTG_STS  USB_OTG_FlushTxFifo     (USB_OTG_CORE_HANDLE *pdev , uint32_t num);
USB_OTG_STS  USB_OTG_FlushRxFifo     (USB_OTG_CORE_HANDLE *pdev);


uint32_t USB_OTG_ReadCoreItr (USB_OTG_CORE_HANDLE *pdev);

// returns the USB_OTG Interrupt register
inline uint32_t USB_OTG_ReadOtgItr (USB_OTG_CORE_HANDLE *pdev) { return (USB_OTG_READ_REG32 (&pdev->regs.GREGS->GOTGINT)); }

uint8_t      USB_OTG_IsHostMode      (USB_OTG_CORE_HANDLE *pdev);
uint8_t      USB_OTG_IsDeviceMode    (USB_OTG_CORE_HANDLE *pdev);
uint32_t     USB_OTG_GetMode         (USB_OTG_CORE_HANDLE *pdev);
USB_OTG_STS  USB_OTG_PhyInit         (USB_OTG_CORE_HANDLE *pdev);

/*********************** HOST APIs ********************************************/
#ifdef USE_HOST_MODE
USB_OTG_STS  USB_OTG_CoreInitHost    (USB_OTG_CORE_HANDLE *pdev);
USB_OTG_STS  USB_OTG_EnableHostInt   (USB_OTG_CORE_HANDLE *pdev);
USB_OTG_STS  USB_OTG_HC_Init         (USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num);
USB_OTG_STS  USB_OTG_HC_Halt         (USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num);
USB_OTG_STS  USB_OTG_HC_StartXfer    (USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num);
USB_OTG_STS  USB_OTG_HC_DoPing       (USB_OTG_CORE_HANDLE *pdev , uint8_t hc_num);
uint32_t     USB_OTG_ReadHostAllChannels_intr    (USB_OTG_CORE_HANDLE *pdev);
uint32_t     USB_OTG_ResetPort       (USB_OTG_CORE_HANDLE *pdev);
uint32_t     USB_OTG_ReadHPRT0       (USB_OTG_CORE_HANDLE *pdev);
void         USB_OTG_DriveVbus       (USB_OTG_CORE_HANDLE *pdev, uint8_t state);
void         USB_OTG_InitFSLSPClkSel (USB_OTG_CORE_HANDLE *pdev ,uint8_t freq);
uint8_t      USB_OTG_IsEvenFrame     (USB_OTG_CORE_HANDLE *pdev) ;
#endif



#endif  /* __USB_CORE_H__ */


/**
  * @}
  */ 

/**
  * @}
  */ 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

