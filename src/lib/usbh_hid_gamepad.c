/**
  ******************************************************************************
  * @file    usbh_hid_gamepad.c 
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   This file is the application layer for USB Host HID Mouse Handling.                  
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
#include "usbh_hid_gamepad.h"



/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */
  
/** @defgroup USBH_HID_GAMEPAD 
  * @brief    This file includes HID Layer Handlers for USB Host HID class.
  * @{
  */ 

/** @defgroup USBH_HID_GAMEPAD_Private_TypesDefinitions
  * @{
  */

typedef struct {
        uint16_t vid, pid, pid2, pid3; // applicable vid & pid for the device. pid2,3 : alternate pids
        // config

        /* dpad type : 
         *   0 : None, use analog
         *       1 : 7 value hat switch 0-7 0-270 degrees
         *       2 : XY Gamepad (2x2bits)
         *       3 : independant UDLR button
         *       ...
         */
        unsigned int dpad_type:3;

        /* analog type : 
         *   0 : None, use dpad
         *       1 : 2x8 bit axis, 0 default
         *       2 : 2x8 bit axis, 127 default 
         */
        unsigned int analog_type:2;
        unsigned int max_button_index:3;  // 8 buttons max . 0-7, must have 1 button!
        unsigned int reserved_config:8;  
           
        /* 
        // IDEA, use 10 bits for location as max packet size is 128 (eg. it's 64)
        // this would enable specifying individual U/D/L/R locations, + 2 extra 
        uint32_t bitpos_2msb;
        uint8_t analog_x;
        uint8_t analog_y;
        uint8_t dpad_bit[4];
        uint8_t button_bit[8];
        uint8_t reserved_bit[2];
        */
        uint16_t dpad_bit; // starting bit, len is taken from type
        uint16_t analog_bit;

        // button descriptor : button byte/bit offset
        // order: B Y select start A X L R
        uint16_t button_bit[8];  // starting bit from start of payload. Max = 65536/8=8k 

        uint16_t reserved_data;   // 32 bytes is a nicely aligned data size
} USB_Gamepad_descriptor;
 
/**
  * @}
  */ 


/** @defgroup USBH_HID_GAMEPAD_Private_Defines
  * @{
  */ 
static const USB_Gamepad_descriptor device_table[] = {
        // Thrustmaster Firestorm - no select/start button
        {.vid=0x044f,.pid=0xb315,.pid2=0xb301,.pid3=0xb300,.dpad_type=1,.analog_type=1,.max_button_index=7,
         .dpad_bit=20,.analog_bit=24,.button_bit={0,1,2,3,4,5,6,7}       
        },
        // Trust GXT 24 
        {.vid=0x0079,.pid=0x0006,.pid2=0x0006,.pid3=0x0006,.dpad_type=1,.analog_type=0,.max_button_index=7,
         .dpad_bit=40,.analog_bit=0,.button_bit={44,45,46,47,48,49,50,51}
        },
        // SNES Gamepad USB, Ebay
        {.vid=0x0079,.pid=0x0011,.pid2=0x0011,.pid3=0x0011,.dpad_type=0,.analog_type=2,.max_button_index=7,
         .dpad_bit=0,.analog_bit=0,.button_bit={44,45,46,47,48,49,50,51}
        },      
        {.vid=0} // terminator
};
/**
  * @}
  */ 


/** @defgroup USBH_HID_GAMEPAD_Private_Macros
  * @{
  */ 
// as reported by device interrupt data
#define MAX_REPORT_SIZE 128

/**
  * @}
  */ 

/** @defgroup USBH_HID_GAMEPAD_Private_FunctionPrototypes
  * @{
  */ 
static inline uint32_t extract(uint8_t *,uint16_t,uint8_t);
static int   GAMEPAD_Detect(uint16_t,uint16_t);
static void  GAMEPAD_Init (uint8_t, uint16_t,uint16_t);
static void  GAMEPAD_Decode(uint8_t, uint8_t *data);
/**
  * @}
  */ 


/** @defgroup USBH_HID_GAMEPAD_Private_Variables
  * @{
  */
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
 #if defined   (__CC_ARM) /*!< ARM Compiler */
  __align(4) 
 #elif defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4
 #elif defined (__GNUC__) /*!< GNU Compiler */
 #pragma pack(4) 
 #elif defined  (__TASKING__) /*!< TASKING Compiler */                           
  __align(4) 
 #endif /* __CC_ARM */
#endif
 
    
// VID = 0 : not configured
const USB_Gamepad_descriptor *gamepad_descriptor[2]; // currently used gamepad table. ID=0 : not configured
HID_GAMEPAD_Data_TypeDef HID_GAMEPAD_Data[2];

// static char gamepad_report[2][MAX_REPORT_SIZE]; 
 
HID_cb_TypeDef HID_GAMEPAD_cb = 
{
  GAMEPAD_Detect,
  GAMEPAD_Init,
  GAMEPAD_Decode,
};
/**
  * @}
  */ 


/** @defgroup USBH_HID_GAMEPAD_Private_Functions
  * @{
  */ 

static const USB_Gamepad_descriptor *GAMEPAD_FindDesc(uint16_t vid, uint16_t pid)
{
  int i = 0;

  while (device_table[i].vid != 0) {
    if (device_table[i].vid == vid && ( 
      device_table[i].pid == pid || 
      device_table[i].pid2 == pid ||  
      device_table[i].pid3 == pid 
      ))  break;
    i++;
  }

  return &device_table[i];
}


static int GAMEPAD_Detect (uint16_t vid, uint16_t pid)
{
  const USB_Gamepad_descriptor *gp = GAMEPAD_FindDesc(vid,pid);

  if (gp->vid == 0) return 0;

  return 1;
}


/**
* @brief  GAMEPAD_Init
*         Init Mouse State.
* @param  None
* @retval None
*/
static void  GAMEPAD_Init (uint8_t coreID, uint16_t vid, uint16_t pid)
{
 /* Call User Init*/
 gamepad_descriptor[coreID] = GAMEPAD_FindDesc(vid,pid);

 // USR_GAMEPAD_Init();
}

static inline uint32_t extract(uint8_t *data, uint16_t bitref, uint8_t nbits)
// extracts the bitref-th bit from data, size wise
// assumes does not cross a 32bit boundary !
// XXX use special instruction (faster) 
{
        uint32_t x = ((uint32_t *)data)[bitref>>5] >> (bitref & 31);
        uint32_t mask = (1<<nbits)-1;
        return x & mask;
}

/**
* @brief  GAMEPAD_Decode
*         Decode Gamepad data
* @param  data : Pointer to Mouse HID data buffer
* @retval None
*/
static void  GAMEPAD_Decode(uint8_t coreID, uint8_t *data)
{
        uint8_t device = coreID; 
        // const data conversion from 0-270 degrees + -1 to x/y coordinates, -1 -> 7
        // diagonals are sqrt(2)/2
        const int8_t button_translate[8] = {gamepad_B,gamepad_Y,gamepad_select,gamepad_start,gamepad_A,gamepad_X,gamepad_L,gamepad_R};
        // msb up/down/left/right lsb
        const int8_t dig_to_analog[16][2] = {{0,0},{127,0},{-127,0},{0,0},{0,127},{89,89},{-89,89},{0,0},{0,-127},{89,-89},{-89,-89},{0,0},{0,0},{0,0},{0,0},{0,0}};
        uint8_t val;

        const USB_Gamepad_descriptor *gp = gamepad_descriptor[coreID]; // shortcut

        if (!gp->vid) return; // not configured

        HID_GAMEPAD_Data[device].button = 0;

        // button
        for (int button=0;button<=gp->max_button_index;button++)
        {
                if (extract(data,gp->button_bit[button],1))
                {
                        HID_GAMEPAD_Data[device].button |= 1<<button_translate[button];
                }
        }

        // Hat switch
        switch(gp->dpad_type)
        {
                case 1 : // 0-7 hat switch + -1 as null value
                        val = extract(data, gp->dpad_bit, 4);
                        switch (val) 
                        {
                          case 0:
                          HID_GAMEPAD_Data[device].button |= 1 << gamepad_up;
                          break;
                          case 1:
                          HID_GAMEPAD_Data[device].button |= (1 << gamepad_up) | (1 << gamepad_right);
                          break;
                          case 2:
                          HID_GAMEPAD_Data[device].button |= 1 << gamepad_right;
                          break;
                          case 3:
                          HID_GAMEPAD_Data[device].button |= (1 << gamepad_down) | (1 << gamepad_right);
                          break;
                          case 4:
                          HID_GAMEPAD_Data[device].button |= 1 << gamepad_down;
                          break;
                          case 5:
                          HID_GAMEPAD_Data[device].button |= (1 << gamepad_down) | (1 << gamepad_left);
                          break;
                          case 6:
                          HID_GAMEPAD_Data[device].button |= 1 << gamepad_left;
                          break;
                          case 7:
                          HID_GAMEPAD_Data[device].button |= (1 << gamepad_up) | (1 << gamepad_left);
                          break;
                          case 15:
                          default:
                            // none pressed
                            break;
                        }
                        break;
                case 2 : // x/y 2 bits -1,0,1 as 11,00,01
                        val= extract(data, gp->dpad_bit, 2);
                        HID_GAMEPAD_Data[device].button |= (val&2)?(1<<gamepad_left):((val&1)?(1<<gamepad_right):0);
                        val= extract(data, gp->dpad_bit+2, 2);
                        HID_GAMEPAD_Data[device].button |= (val&2)?(1<<gamepad_down):((val&1)?(1<<gamepad_up):0);
                        break;
                case 3 :
                        val= extract(data, gp->dpad_bit, 4);
                        HID_GAMEPAD_Data[device].button |= (val << gamepad_right);
                        break;

        }

        switch (gp->analog_type) 
        {
                case 1 :
                        HID_GAMEPAD_Data[device].x = (int8_t)extract(data, gp->analog_bit, 8);
                        HID_GAMEPAD_Data[device].y = (int8_t)extract(data, gp->analog_bit+8, 8);
                        break;
                case 2 :
                        HID_GAMEPAD_Data[device].x = (int16_t)extract(data, gp->analog_bit, 8) - 127;
                        HID_GAMEPAD_Data[device].y = (int16_t)extract(data, gp->analog_bit+8, 8) - 127;
                        break;
                case 0 :
                default:
                        val = (HID_GAMEPAD_Data[device].button >> gamepad_right) & 0x0F;
                        HID_GAMEPAD_Data[device].x = dig_to_analog[val][0];
                        HID_GAMEPAD_Data[device].y = dig_to_analog[val][1];
                        break;
        }

        if (gp->dpad_type == 0) {
          if (HID_GAMEPAD_Data[device].x >= 63) {
            HID_GAMEPAD_Data[device].button |= 1 << gamepad_right;
          } else if (HID_GAMEPAD_Data[device].x <= -63) {
            HID_GAMEPAD_Data[device].button |= 1 << gamepad_left;
          }
          
          if (HID_GAMEPAD_Data[device].y >= 63) {
            HID_GAMEPAD_Data[device].button |= 1 << gamepad_up;
          } else if (HID_GAMEPAD_Data[device].y <= -63) {
            HID_GAMEPAD_Data[device].button |= 1 << gamepad_down;
          }     
        }

    // Comment this line to get accurate timing of the parser, this only lits leds:
    // USR_GAMEPAD_ProcessData(&HID_GAMEPAD_Data[device]);

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


/**
  * @}
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
