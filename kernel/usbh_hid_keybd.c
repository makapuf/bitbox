/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_devices.h"

#include <string.h>

#define  KBR_MAX_NBR_PRESSED 6

static int  KEYBRD_Detect (uint16_t,uint16_t);
static void  KEYBRD_Init (uint8_t,uint16_t,uint16_t);
static void  KEYBRD_Decode(uint8_t,uint8_t *data);


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #pragma pack(4) 
#endif

volatile uint8_t keyboard_mod[2]; // LCtrl =1, LShift=2, LAlt=4, LWin - Rctrl, ...
volatile uint8_t keyboard_key[6][2]; // using raw USB key codes


HID_cb_TypeDef HID_KEYBRD_cb= 
{
    KEYBRD_Detect,
    KEYBRD_Init,
    KEYBRD_Decode
};


static int  KEYBRD_Detect (uint16_t vid, uint16_t pid)
{
    return 1;
}


static void KEYBRD_Init (uint8_t coreID, uint16_t vid, uint16_t pid)
{
    device_type[coreID]=device_keyboard;
}


/* This emulates the gamepad with a keyboard when plugged
 * mapping:

    Space : Select,   2C
    Enter : Start,    28
    UDLR arrows : D-pad    52, 51, 50, 4F
    D     : A button, 07
    F : B button, 09
    E : X button, 08
    R : Y button, 15
    Left/Right CTRL (L/R shoulders) -> mods!
 */

static void kbd_emulate_gamepad (void)
{
    // kbd code for each gamepad buttons 
    static const uint8_t kbd_gamepad[] = {
        0x07, 0x09, 0x08, 0x15, 0xe0, 0xe4, 0x2c, 0x28, 0x52, 0x51, 0x50, 0x4f
    };

    gamepad_buttons[0]=0;
    for (int i=0;i<sizeof(kbd_gamepad);i++) {
        if (memchr((char *)keyboard_key[0],kbd_gamepad[i],KBR_MAX_NBR_PRESSED))
            gamepad_buttons[0]|= (1<<i);
    }

    // special : mods
    if (keyboard_mod[0] & 1) 
        gamepad_buttons[0] |= gamepad_L;
    if (keyboard_mod[0] & 16) 
        gamepad_buttons[0] |= gamepad_R;

}


/* boot keyboard report  : 
        u8 modifier (LCtrl =1, LShift=2, LAlt=4, LWin - Rctrl, ... )
        u8 reserved
        8u codes[6] of pressed keys
*/
static void KEYBRD_Decode(uint8_t coreID, uint8_t *pbuf)
{
    keyboard_mod[coreID]=pbuf[0];
    memcpy((char *)keyboard_key[coreID],pbuf+2,KBR_MAX_NBR_PRESSED);
    kbd_emulate_gamepad();
}

