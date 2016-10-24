/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_devices.h"


static int MOUSE_Detect (uint16_t, uint16_t);
static void  MOUSE_Init (uint8_t, uint16_t, uint16_t);
static void  MOUSE_Decode(uint8_t, uint8_t *data);

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
	#pragma pack(4) 
#endif
 
    
// global mouse state 
volatile int8_t mouse_x;
volatile int8_t mouse_y;
volatile uint8_t  mouse_buttons = 0;
 
// struct _HID_MOUSE_Data HID_MOUSE_Data;
HID_cb_TypeDef HID_MOUSE_cb = 
{
	MOUSE_Detect,
	MOUSE_Init,
	MOUSE_Decode,
};


static int MOUSE_Detect (uint16_t vid,uint16_t pid)
{
	/* Call User Init*/
	return 1;
}

static void  MOUSE_Init (uint8_t coreID, uint16_t vid, uint16_t pid)
{
    // do nothing but signals mouse initialized
    device_type[coreID]=device_mouse;
}
            

static void  MOUSE_Decode(uint8_t coreID, uint8_t *data)
{
	// user process, no clip
	mouse_x = (int8_t)data[1];
	mouse_y = (int8_t)data[2];  
	mouse_buttons = data[0];
}
