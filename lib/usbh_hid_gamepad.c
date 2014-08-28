#include "usbh_hid_devices.h"

// for now no event is fired for joysticks
volatile uint16_t gamepad_buttons[2]; // simple mapping : ABXY LR Start Select UDLR xxxx
volatile int8_t gamepad_x[2], gamepad_y[2]; // analog pad values

typedef struct {
    uint16_t vid, pid, pid2, pid3; // applicable vid & pid for the device. pid2,3 : alternate pids
    // config

    /* hat type : 
     *   0 : None, use analog
     *   1 : 7 value hat switch 0-7 0-270 degrees (or 0-315)
     *   2 : LDRU individual bits
     */
    unsigned int dpad_type:3;

    /* analog type : 
     *   0 : None, use dpad, XY values not used
     *   1 : 2x8 bit axis, 0 default 
     *   2 : 2x8 bit axis, 127 default 
     */
    unsigned int analog_type:2;
    unsigned int max_button_index:3;  // 8 buttons max . 0-7, must have 1 button!
    unsigned int reserved_config:8;  
         
    uint16_t dpad_bit; // starting bit, len is taken from type
    uint16_t analog_X_bit; // starting bit
    uint16_t analog_Y_bit;

    // button descriptor : button byte/bit offset
    // order: B Y select start A X L R
    uint16_t button_bit[8];  // starting bit from start of payload. Max = 65536/8=8k 

    uint16_t reserved_data;   // 32 bytes is a nicely aligned data size
} USB_Gamepad_descriptor;

// static table Gamepads definition. 
// Allows us to avoid implementing a full descriptor parser.
static const USB_Gamepad_descriptor device_table[] = {
    // Thrustmaster Firestorm - no select/start button
    {
        .vid=0x044f,.pid=0xb315,.pid2=0xb301,.pid3=0xb300,
        .dpad_type=1,.analog_type=1,.max_button_index=7,
        .dpad_bit=20,.analog_X_bit=24,.analog_Y_bit=32,
        .button_bit={0,1,2,3,4,5,6,7}       
    },

    // Thrustmaster T-Wireless (FIXME : correct start/select )
    {
        .vid=0x044f,.pid=0xd007,
        .dpad_type=1,.analog_type=1,.max_button_index=7,
        .dpad_bit=20,.analog_X_bit=24,.analog_Y_bit=32,
        .button_bit={0,1,2,3,4,5,6,7}       
    },

    // Trust GXT 24 
    {
        .vid=0x0079,.pid=0x0006,.pid2=0x0006,.pid3=0x0006,
        .dpad_type=1,.analog_type=0,.max_button_index=7,
        .dpad_bit=40,
        .button_bit={44,45,46,47,48,49,50,51}
    },

    // SNES Gamepad USB, Ebay
    {
        .vid=0x0079,.pid=0x0011,.pid2=0x0011,.pid3=0x0011,
        .dpad_type=0,.analog_type=2,.max_button_index=7,
        .dpad_bit=0,
        .button_bit={44,45,46,47,48,49,50,51}
    },     

    // PS3 dualshock 3 (info from http://eleccelerator.com/wiki/index.php?title=DualShock_3)
    {
        .vid=0x054C, .pid=0x0268, 
        .dpad_type=2, .analog_type=3, .max_button_index=7,
        .dpad_bit=2*8+3,
        .analog_X_bit=40, .analog_Y_bit=48,
        .button_bit={16+14,16+15,16+0,16+3,16+13,16+12,16+11,16+10} //  B=X Y=square select start A=O X=tri L R
    },

    // XBOX 360 wireless : see info from http://free60.org/wiki/index.php?title=GamePad

    {.vid=0} // terminator
};

// as reported by device interrupt data
#define MAX_REPORT_SIZE 128

static inline uint32_t extract(uint8_t *,uint16_t,uint8_t);
static int   GAMEPAD_Detect(uint16_t,uint16_t);
static void  GAMEPAD_Init (uint8_t, uint16_t,uint16_t);
static void  GAMEPAD_Decode(uint8_t, uint8_t *data);

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
 #pragma pack(4) 
#endif
        
// VID = 0 : not configured
const USB_Gamepad_descriptor *gamepad_descriptor[2]; // currently used gamepad table. ID=0 : not configured

HID_cb_TypeDef HID_GAMEPAD_cb = 
{
    GAMEPAD_Detect,
    GAMEPAD_Init,
    GAMEPAD_Decode,
};


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

static void  GAMEPAD_Init (uint8_t coreID, uint16_t vid, uint16_t pid)
{
    gamepad_descriptor[coreID] = GAMEPAD_FindDesc(vid,pid);

    struct event e;
    e.type = evt_device_change;
    e.device.port = coreID;
    e.device.type = device_gamepad;
    event_push(e);
}

static inline uint32_t extract(uint8_t *data, uint16_t bitref, uint8_t nbits)
// extracts the bitref-th bit from data, size wise
// assumes does not cross a 32bit boundary !
{
    uint32_t x = ((uint32_t *)data)[bitref>>5] >> (bitref & 31);
    uint32_t mask = (1<<nbits)-1;
    return x & mask;
}

// data : Pointer to Mouse HID data buffer
static void GAMEPAD_Decode(uint8_t coreID, uint8_t *data)
{
    uint8_t val;

    const uint16_t hat_translate[16] = {
        gamepad_up,
        gamepad_up | gamepad_right,
        gamepad_right,
        gamepad_right | gamepad_down,
        gamepad_down,
        gamepad_down | gamepad_left,
        gamepad_left,
        gamepad_left | gamepad_up,
    }; // rest is zero, included 15 which means position neutral

    // shortcut
    const USB_Gamepad_descriptor *gp = gamepad_descriptor[coreID]; 

    if (!gp->vid) return; // not configured

    gamepad_buttons[coreID] = 0;

    // Button
    for (int button=0;button<=gp->max_button_index;button++)
        if (extract(data,gp->button_bit[button],1))
            gamepad_buttons[coreID] |= 1<<button;

    // Hat switch
    switch(gp->dpad_type)
    {
        case 1 : // 0-7 hat switch + -1 as null value
            val = extract(data, gp->dpad_bit, 4);
            gamepad_buttons[coreID] |= hat_translate[val];             
        break;

        case 2 : // LDRU individual bits (ps3)
            val = extract(data, gp->dpad_bit, 4);
            if (val & 1)
                gamepad_buttons[coreID] |= gamepad_left;
            if (val & 2)
                gamepad_buttons[coreID] |= gamepad_down;
            if (val & 4)
                gamepad_buttons[coreID] |= gamepad_right;
            if (val & 8)
                gamepad_buttons[coreID] |= gamepad_up;
        break;

        case 0 : 
        default : 
            // do nothing
        break;
    }

    // analog
    switch (gp->analog_type) 
    {
        case 1 :
            gamepad_x[coreID] = (int8_t)extract(data, gp->analog_X_bit, 8);
            gamepad_y[coreID] = (int8_t)extract(data, gp->analog_Y_bit, 8);
            break;
        case 2 :
            gamepad_x[coreID] = (int16_t)extract(data, gp->analog_X_bit, 8) - 127;
            gamepad_y[coreID] = (int16_t)extract(data, gp->analog_Y_bit, 8) - 127;
            break;
        case 3 : 

            break;
        case 0 :
        default:
            // do nothing
            break;
    }
}
