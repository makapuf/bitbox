/**
  ******************************************************************************
  * @file    usbh_usr.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   This file includes the user application layer
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
//#include "lcd_log.h"
#include "usbh_usr.h"
#include "usbh_hid_mouse.h"
#include "usbh_hid_gamepad.h"
#include "usbh_hid_keybd.h"
#include "stm32f4xx_gpio.h"

/** @addtogroup USBH_USER
* @{
*/

/** @addtogroup USBH_HID_DEMO_USER_CALLBACKS
* @{
*/

/** @defgroup USBH_USR
* @brief This file is the Header file for usbh_usr.c
* @{
*/ 


/** @defgroup USBH_CORE_Exported_TypesDefinitions
* @{
*/ 
#define KYBRD_FIRST_COLUMN               (uint16_t)319
#define KYBRD_LAST_COLUMN                (uint16_t)7
#define KYBRD_FIRST_LINE                 (uint8_t)120
#define KYBRD_LAST_LINE                  (uint8_t)200

////////////////////////////////////////////////
volatile uint8_t  data_X = 0;
volatile uint8_t  data_Y = 0;
volatile uint8_t  data_Button = 0;  
uint8_t  data_from_keyboard[50];
volatile uint8_t keyboard_idx=0;
///////////////////////////////////////////////

__IO uint8_t USBH_Usr_InputDone = 0;

/**
* @}
*/ 
uint8_t  KeybrdCharXpos           = 0;
uint16_t KeybrdCharYpos           = 0;
extern  int16_t  x_loc, y_loc; 
extern __IO int16_t  prev_x, prev_y;

/** @addtogroup USER
* @{
*/

/** @defgroup USBH_USR 
* @brief    This file includes the user application layer
* @{
*/ 

/** @defgroup USBH_CORE_Exported_Types
* @{
*/ 



/** @defgroup USBH_USR_Private_Defines
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Macros
* @{
*/ 
/**
* @}
*/ 

/** @defgroup USBH_USR_Private_Variables
* @{
*/
//extern USB_OTG_CORE_HANDLE           USB_OTG_Core_dev;
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

USBH_Usr_cb_TypeDef USR_Callbacks =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  NULL,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
};

/**
* @}
*/

/** @defgroup USBH_USR_Private_Constants
* @{
*/ 
/*--------------- LCD Messages ---------------*/
//const uint8_t MSG_HOST_INIT[]          = "> Host Library Initialized\n";
//const uint8_t MSG_DEV_ATTACHED[]       = "> Device Attached\n";
//const uint8_t MSG_DEV_DISCONNECTED[]   = "> Device Disconnected\n";
//const uint8_t MSG_DEV_ENUMERATED[]     = "> Enumeration completed\n";
//const uint8_t MSG_DEV_HIGHSPEED[]      = "> High speed device detected\n";
//const uint8_t MSG_DEV_FULLSPEED[]      = "> Full speed device detected\n";
//const uint8_t MSG_DEV_LOWSPEED[]       = "> Low speed device detected\n";
//const uint8_t MSG_DEV_ERROR[]          = "> Device fault \n";
//
//const uint8_t MSG_MSC_CLASS[]          = "> Mass storage device connected\n";
//const uint8_t MSG_HID_CLASS[]          = "> HID device connected\n";
//
//const uint8_t USB_HID_MouseStatus[]    = "> HID Demo Device : Mouse\n";
//const uint8_t USB_HID_KeybrdStatus[]   = "> HID Demo Device : Keyboard\n";
//const uint8_t MSG_UNREC_ERROR[]        = "> UNRECOVERED ERROR STATE\n";
/**
* @}
*/



/** @defgroup USBH_USR_Private_FunctionPrototypes
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Functions
* @{
*/ 





/**
* @brief  USBH_USR_Init 
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBH_USR_Init(void)
{
  static uint8_t startup = 0;  
    GPIO_InitTypeDef  GPIO_InitStructure;
  
  if(startup == 0 )
  {
    startup = 1;
    USBH_Usr_InputDone = 0;
  /* Enable the GPIO_LED Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
    

    /* Configure the LEDs */
/*
    STM_EVAL_LEDInit(LED_Green);
    STM_EVAL_LEDInit(LED_Orange);
    STM_EVAL_LEDInit(LED_Red); 
    STM_EVAL_LEDInit(LED_Blue); 
    
    STM_EVAL_LEDOn(LED_Green);
    STM_EVAL_LEDOn(LED_Orange);
    STM_EVAL_LEDOn(LED_Red);
    STM_EVAL_LEDOn(LED_Blue);
    
    STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);
*/
/*    
#if defined (USE_STM322xG_EVAL)
  STM322xG_LCD_Init();
#elif defined(USE_STM324xG_EVAL)
  STM324xG_LCD_Init();
#elif defined (USE_STM3210C_EVAL)
  STM3210C_LCD_Init();
#else
 #error "Missing define: Evaluation board (ie. USE_STM322xG_EVAL)"
#endif

    LCD_LOG_Init();

#ifdef USE_USB_OTG_HS 
    LCD_LOG_SetHeader(" USB OTG HS HID Host");
#else
    LCD_LOG_SetHeader(" USB OTG FS HID Host");
#endif
    LCD_UsrLog("> USB Host library started.\n"); 
    LCD_LOG_SetFooter ("     USB Host Library v2.1.0" );
*/    
  }
}

/**
* @brief  USBH_USR_DeviceAttached 
*         Displays the message on LCD on device attached
* @param  None
* @retval None
*/
void USBH_USR_DeviceAttached(void)
{  
  //LCD_UsrLog ((void*)MSG_DEV_ATTACHED);
}

/**
* @brief  USBH_USR_UnrecoveredError
* @param  None
* @retval None
*/
void USBH_USR_UnrecoveredError (void)
{
  //LCD_ErrLog((void*)MSG_UNREC_ERROR);
}

/**
* @brief  USBH_DisconnectEvent
*         Device disconnect event
* @param  None
* @retval None
*/
void USBH_USR_DeviceDisconnected (void)
{
  
  //LCD_SetBackColor(Black); 
  //LCD_SetTextColor(Black);   
  
  //LCD_LOG_ClearTextZone();

  //LCD_DrawRect(MOUSE_WINDOW_X,
  //             MOUSE_WINDOW_Y, 
  //             MOUSE_WINDOW_HEIGHT,
  //             MOUSE_WINDOW_WIDTH);
  //LCD_SetTextColor(White);    
  
  //LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 42, "                                   ");
  //LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 30, "                                   ");  
  
  //LCD_ErrLog((void *)MSG_DEV_DISCONNECTED);
}

/**
* @brief  USBH_USR_ResetUSBDevice 
*         Reset USB Device
* @param  None
* @retval None
*/
void USBH_USR_ResetDevice(void)
{
  /* Users can do their application actions here for the USB-Reset */
}


/**
* @brief  USBH_USR_DeviceSpeedDetected 
*         Displays the message on LCD for device speed
* @param  Devicespeed : Device Speed
* @retval None
*/
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
//  if(DeviceSpeed == HPRT0_PRTSPD_HIGH_SPEED)
//  {
////   LCD_UsrLog((void *)MSG_DEV_HIGHSPEED);
//  }
//  else if(DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED)
//  {
////LCD_UsrLog((void *)MSG_DEV_FULLSPEED);
//  }
//  else if(DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED)
//  {
////    LCD_UsrLog((void *)MSG_DEV_LOWSPEED);
//  }
//  else
//  {
////    LCD_UsrLog((void *)MSG_DEV_ERROR);
//  }
}

/**
* @brief  USBH_USR_Device_DescAvailable 
*         Displays the message on LCD for device descriptor
* @param  DeviceDesc : device descriptor
* @retval None
*/
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
//  uint8_t temp[50];
//  USBH_DevDesc_TypeDef *hs;
//  hs = DeviceDesc;
//
//
//  sprintf((char *)temp , "VID : %04Xh\n" , (uint32_t)(*hs).idVendor); /
//  //LCD_UsrLog((void *)temp);
//
//
//  sprintf((char *)temp , "PID : %04Xh\n" , (uint32_t)(*hs).idProduct);
//  //LCD_UsrLog((void *)temp);
}

/**
* @brief  USBH_USR_DeviceAddressAssigned 
*         USB device is successfully assigned the Address 
* @param  None
* @retval None
*/
void USBH_USR_DeviceAddressAssigned(void)
{
  
}


/**
* @brief  USBH_USR_Conf_Desc 
*         Displays the message on LCD for configuration descriptor
* @param  ConfDesc : Configuration descriptor
* @retval None
*/
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc)
{
//  USBH_InterfaceDesc_TypeDef *id;
//
//  id = itfDesc;
//
//  if((*id).bInterfaceClass  == 0x08)
//  {
////    LCD_UsrLog((void *)MSG_MSC_CLASS);
//  }
//  else if((*id).bInterfaceClass  == 0x03)
//  {
////    LCD_UsrLog((void *)MSG_HID_CLASS);
//  }
}

/**
* @brief  USBH_USR_Manufacturer_String 
*         Displays the message on LCD for Manufacturer String 
* @param  ManufacturerString : Manufacturer String of Device
* @retval None
*/
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
//  char temp[100];
//  sprintf(temp, "Manufacturer : %s\n", (char *)ManufacturerString);
////  LCD_UsrLog((void *)temp);
  
}

/**
* @brief  USBH_USR_Product_String 
*         Displays the message on LCD for Product String
* @param  ProductString : Product String of Device
* @retval None
*/
void USBH_USR_Product_String(void *ProductString)
{
//  char temp[100];
//  sprintf((char *)temp, "Product : %s\n", (char *)ProductString);
////  LCD_UsrLog((void *)temp);
  
}

/**
* @brief  USBH_USR_SerialNum_String 
*         Displays the message on LCD for SerialNum_String 
* @param  SerialNumString : SerialNum_String of device
* @retval None
*/
void USBH_USR_SerialNum_String(void *SerialNumString)
{
//  uint8_t temp[100];
//  sprintf((char *)temp, "Serial Number : %s\n", (char *)SerialNumString);
////  LCD_UsrLog((void *)temp);
} 

/**
* @brief  EnumerationDone 
*         User response request is displayed to ask for
*         application jump to class
* @param  None
* @retval None
*/
void USBH_USR_EnumerationDone(void)
{
  /* Enumeration complete */
  //LCD_UsrLog((void *)MSG_DEV_ENUMERATED);
  //LCD_SetTextColor(Green);
  //LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 42, "To start the HID class operations: " );
  //LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 30, "Press Key...                       ");
  //LCD_SetTextColor(LCD_LOG_DEFAULT_COLOR);   
} 

/**
* @brief  USBH_USR_DeviceNotSupported
*         Device is not supported
* @param  None
* @retval None
*/
void USBH_USR_DeviceNotSupported(void)
{
//  LCD_UsrLog ("> Device not supported.\n");              
  
}  


/**
* @brief  USBH_USR_UserInput
*         User Action for application state entry
* @param  None
* @retval USBH_USR_Status : User response for key button
*/
USBH_USR_Status USBH_USR_UserInput(void)
{
  
  USBH_USR_Status usbh_usr_status;
  
  usbh_usr_status = USBH_USR_NO_RESP;  

  GPIOD->BSRRL = GPIO_Pin_15;
//  STM_EVAL_LEDOn(LED_Blue); // added by "STM32"
  
  // commented by "STM32"
//  /*Key B3 is in polling mode to detect user action */
//  if(STM_EVAL_PBGetState(Button_WAKEUP) == SET) 
//  {

  USBH_Usr_InputDone = 1;    
    usbh_usr_status = USBH_USR_RESP_OK;
    
//  }
  
  
  return usbh_usr_status;
  
} 

/**
* @brief  USBH_USR_OverCurrentDetected
*         Device Overcurrent detection event
* @param  None
* @retval None
*/
void USBH_USR_OverCurrentDetected (void)
{
//   LCD_ErrLog ("Overcurrent detected.\n");
  
}

/**
* @brief  USBH_USR_DeInit
*         Deint User state and associated variables
* @param  None
* @retval None
*/
void USBH_USR_DeInit(void)
{
}

/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/

/**
* @}
*/
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

