#pragma once

/* Includes ------------------------------------------------------------------*/
// will define which USB core is used
#include "kconf.h"
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

#ifdef USE_USB_OTG_HS 
 #define USB_OTG_HS_CORE
#endif

#ifdef USE_USB_OTG_FS 
 #define USB_OTG_FS_CORE
#endif
/*******************************************************************************
*                     FIFO Size Configuration in Host mode
*  
*  (i) Receive data FIFO size = (Largest Packet Size / 4) + 1 or 
*                             2x (Largest Packet Size / 4) + 1,  If a 
*                             high-bandwidth channel or multiple isochronous 
*                             channels are enabled
*
*  (ii) For the host nonperiodic Transmit FIFO is the largest maximum packet size 
*      for all supported nonperiodic OUT channels. Typically, a space 
*      corresponding to two Largest Packet Size is recommended.
*
*  (iii) The minimum amount of RAM required for Host periodic Transmit FIFO is 
*        the largest maximum packet size for all supported periodic OUT channels.
*        If there is at least one High Bandwidth Isochronous OUT endpoint, 
*        then the space must be at least two times the maximum packet size for 
*        that channel.
*******************************************************************************/

/****************** USB OTG HS CONFIGURATION **********************************/
#ifdef USB_OTG_HS_CORE
 #define RX_FIFO_HS_SIZE                          128
 #define TXH_NP_HS_FIFOSIZ                        96
 #define TXH_P_HS_FIFOSIZ                         96

// #define USB_OTG_HS_LOW_PWR_MGMT_SUPPORT
// #define USB_OTG_HS_SOF_OUTPUT_ENABLED

// #define USB_OTG_INTERNAL_VBUS_ENABLED
#define USB_OTG_EXTERNAL_VBUS_ENABLED

 #ifdef USE_ULPI_PHY
  #define USB_OTG_ULPI_PHY_ENABLED
 #endif
 #ifdef USE_EMBEDDED_PHY
   #define USB_OTG_EMBEDDED_PHY_ENABLED
 #endif
 // #define USB_OTG_HS_INTERNAL_DMA_ENABLED
// #define USB_OTG_HS_DEDICATED_EP1_ENABLED
#endif

/****************** USB OTG FS CONFIGURATION **********************************/
#ifdef USB_OTG_FS_CORE
 #define RX_FIFO_FS_SIZE                          128
 #define TXH_NP_FS_FIFOSIZ                         96
 #define TXH_P_FS_FIFOSIZ                          96

// #define USB_OTG_FS_LOW_PWR_MGMT_SUPPORT
// #define USB_OTG_FS_SOF_OUTPUT_ENABLED
#endif

/****************** USB OTG MODE CONFIGURATION ********************************/
#define USE_HOST_MODE
//#define USE_DEVICE_MODE
//#define USE_OTG_MODE



/****************** C Compilers dependant keywords ****************************/
/* In HS mode and when the DMA is used, all variables and data structures dealing
   with the DMA during the transaction process should be 4-bytes aligned */    
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #define __ALIGN_END    __attribute__ ((aligned (4)))
  #define __ALIGN_BEGIN         
#else
  #define __ALIGN_BEGIN
  #define __ALIGN_END   
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/* __packed keyword used to decrease the data type alignment to 1-byte */
#define __packed    __attribute__ ((__packed__)) 



