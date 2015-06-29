/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_devices.h"

#define  KBR_MAX_NBR_PRESSED 6

static int  KEYBRD_Detect (uint16_t,uint16_t);
static void  KEYBRD_Init (uint8_t,uint16_t,uint16_t);
static void  KEYBRD_Decode(uint8_t,uint8_t *data);


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
 #pragma pack(4) 
#endif
 

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
        struct event e;
        e.type = evt_device_change;    // do nothing but signals mouse initialized
        e.device.port = coreID;
        e.device.type = device_keyboard;
        event_push(e);
        // XXX change static devices type
}


/* boot keyboard report  : 
        u8 modifier (LCtrl =1, LShift=2, LAlt=4, LWin - Rctrl, ... )
        u8 reserved
        8u codes[6] of pressed keys
*/

char kbd_map(uint8_t mod, uint8_t key); // evt_queue.c

static void KEYBRD_Decode(uint8_t coreID, uint8_t *pbuf)
{
        static uint8_t nb_last;
        static uint8_t keys_last[KBR_MAX_NBR_PRESSED]; // list of 0-nb_last codes of last pressed keys
        static uint8_t mod_last;

        int j;
        struct event e;

        // keyboard boot protocol : modifiers bits(Ctrl, Shift, Alt, Win L then R), reserved, char[6]

        // calculates New-Last & send keypresses
        for (int i = 2; i < 2 + KBR_MAX_NBR_PRESSED; i++) {                       
                if (pbuf[i]==0) continue; // break?
                // tests for errors (code 1,2 or 3)
                if ((unsigned)(pbuf[i]-1) <= 2)
                        return;

                for (j=0; j<nb_last;j++) 
                        if (pbuf[i]==keys_last[j]) 
                                break;
                

                // not found ?
                if (j==nb_last) 
                {
                        e.type=evt_keyboard_press;
                        e.kbd.mod=pbuf[0];
                        e.kbd.key=pbuf[i];
                        e.kbd.sym=kbd_map(pbuf[0],pbuf[i]);
                        event_push(e);
                }
        }

        
        // now calculates old-new and send key released events.
        for (int i=0;i<nb_last;i++)
        {
                for (j=0;j<KBR_MAX_NBR_PRESSED;j++)
                        if (pbuf[2+j]==keys_last[i])
                                break;
                // not found
                if (j==KBR_MAX_NBR_PRESSED)
                {
                        e.type=evt_keyboard_release;
                        e.kbd.mod=pbuf[0];
                        e.kbd.key=keys_last[i];
                        e.kbd.sym=kbd_map(pbuf[0],keys_last[i]);
                        event_push(e);
                }                
        }

        // special case : modifier keys as keypresses
        if (pbuf[0] != mod_last)
            for (int i=0;i<8;i++) {
                if ((pbuf[0] & ~mod_last) & (1<<i)) // new ones
                {
                    e.type=evt_keyboard_press;
                    e.kbd.key=0xE0 + i; // codes are in the same order as bits                
                    event_push(e);
                }

                if ((mod_last & ~pbuf[0]) & (1<<i)) // released ones
                {
                    e.type=evt_keyboard_release;
                    e.kbd.key=0xE0 + i; // codes are in the same order as bits                
                    event_push(e);
                }
            }


        // fills old keys with current 
        nb_last=0;
        for (int i=0;i<KBR_MAX_NBR_PRESSED;i++) {
                if (pbuf[2+i])
                        keys_last[nb_last++]=pbuf[2+i];
        }
        mod_last=pbuf[0];
        
}
