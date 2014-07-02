/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_devices.h"


static int MOUSE_Detect (uint16_t, uint16_t);
static void  MOUSE_Init (uint8_t, uint16_t, uint16_t);
static void  MOUSE_Decode(uint8_t, uint8_t *data);

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
 #pragma pack(4) 
#endif
 
    
// global mouse state 
volatile int data_mouse_x;
volatile int data_mouse_y;
volatile uint8_t  data_mouse_buttons = 0;
 
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
    struct event e;
    e.type = evt_device_change;    // do nothing but signals mouse initialized
    e.device.port = coreID;
    e.device.type = device_mouse;
    event_push(e);
}
            

static void  MOUSE_Decode(uint8_t coreID, uint8_t *data)
{
  static uint8_t old_buttons;
  struct event e;
  
  if (data[1] || data[2])
  {
    // mouse moved
    e.type = evt_mouse_move;    
    e.mov.port = coreID;
    e.mov.x = data[1];
    e.mov.y = data[2];
    event_push(e);
  }

  for (int i=0;i<8;i++) {
    if ((data[0]&~old_buttons) & 1<<i ) // new and not old
    {
      e.type=evt_mouse_click;
      e.button.port = coreID;
      e.button.id = i;
      event_push(e);
    }
    if ((old_buttons & ~data[0]) & 1<<i ) // new and not old
    {
      e.type=evt_mouse_release;
      e.button.port = coreID;
      e.button.id = i;
      event_push(e);
    }
  }

  old_buttons = data[0];


  // user process, no clip
  data_mouse_x += (int8_t) data[1];
  data_mouse_y += (int8_t) data[2];  
  data_mouse_buttons = data[0];
}
