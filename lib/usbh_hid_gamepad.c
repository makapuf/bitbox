#include "usbh_hid_devices.h"

#define THRESHOLD_HAT 30 // value above which an analog is considered commited

// for now no event is fired for joysticks
volatile uint16_t gamepad_buttons[2]; // simple mapping : ABXY LR Start Select UDLR xxxx
volatile int8_t gamepad_x[2], gamepad_y[2]; // analog pad values


USB_Gamepad_descriptor gamepad_parsed_descriptor[2]; // ram descriptors, parsed from the report descriptor, can be overriden

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
 #pragma pack(4) 
#endif
        
// VID = 0 : not configured
const USB_Gamepad_descriptor *gamepad_descriptor[2]; // currently used gamepad table. ID=0 : not configured



// static table Gamepads definition. 
// Allows us to avoid implementing a full descriptor parser.
static const USB_Gamepad_descriptor device_table[] = {
    
    // Thrustmaster Firestorm - no select/start button
    {
        .vid=0x044f,.pid=0xb315,.pid2=0xb301,.pid3=0xb300,
        .dpad_type=0,.analog_type=1,.max_button_index=7,
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

/*    // SNES Gamepad USB, Ebay
    {
        .vid=0x0079,.pid=0x0011,.pid2=0x0011,.pid3=0x0011,
        .dpad_type=0,.analog_type=2,.max_button_index=7,
        .dpad_bit=0,
        .button_bit={44,45,46,47,48,49,50,51}
    },     
*/

    // PS3 dualshock 3 (info from http://eleccelerator.com/wiki/index.php?title=DualShock_3)
    {
        .vid=0x054C, .pid=0x0268, 
        .dpad_type=2, .analog_type=3, .max_button_index=7,
        .dpad_bit=2*8+3,
        .analog_X_bit=40, .analog_Y_bit=48,
        .button_bit={16+14,16+15,16+0,16+3,16+13,16+12,16+11,16+10} //  B=X Y=square select start A=O X=tri L R
    },

    // Logitech Precision Gamepad (by pulkomandy)
    {
        .vid=0x046d, .pid=0xc21a,
        .dpad_type=0,.analog_type=2,.max_button_index=7,
        .dpad_bit=0,
        .analog_X_bit=0, .analog_Y_bit=8,
        .button_bit={17,16,24,25,18,19,20,21} // L2=22 R2=23 
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
    /*
    const USB_Gamepad_descriptor *gp = GAMEPAD_FindDesc(vid,pid);
    if (gp->vid == 0) return 0;
    */
    // shall return 1 only if gamepad descriptor is OK - but we dont have it ...
    return 1;
}

static void  GAMEPAD_Init (uint8_t coreID, uint16_t vid, uint16_t pid)
{
    gamepad_descriptor[coreID] = GAMEPAD_FindDesc(vid,pid);
    if (gamepad_descriptor[coreID]->vid ==0) // not found in table
        gamepad_descriptor[coreID] = &gamepad_parsed_descriptor[coreID];

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

    // Analog
    switch (gp->analog_type) 
    {
        case 1 :
            gamepad_x[coreID] = (int8_t)extract(data, gp->analog_X_bit, 8);
            gamepad_y[coreID] = (int8_t)extract(data, gp->analog_Y_bit, 8);
            break;
        case 2 :
            gamepad_x[coreID] = (int8_t)extract(data, gp->analog_X_bit, 8) - 128;
            gamepad_y[coreID] = (int8_t)extract(data, gp->analog_Y_bit, 8) - 128;
            // little tweak to force "almost zero" (but negative) to zero for rest values.
            if (gamepad_x[coreID] == -1) gamepad_x[coreID] = 0;
            if (gamepad_y[coreID] == -1) gamepad_y[coreID] = 0;
            break;
        case 0 :
        default:
            // do nothing
            break;
    }

    // Buttons
    gamepad_buttons[coreID] = 0;

    // XYABLR Buttons
    for (int button=0;button<=gp->max_button_index;button++)
        if (extract(data,gp->button_bit[button],1))
            gamepad_buttons[coreID] |= 1<<button;

    // Hat switch
    switch(gp->dpad_type)
    {
        case 0 : // Fake it from signed XY values
            if (gamepad_x[coreID]<-THRESHOLD_HAT) 
                gamepad_buttons[coreID] |= gamepad_left;
            if (gamepad_x[coreID]> THRESHOLD_HAT) 
                gamepad_buttons[coreID] |= gamepad_right;

            if (gamepad_y[coreID]<-THRESHOLD_HAT) 
                gamepad_buttons[coreID] |= gamepad_up;
            if (gamepad_y[coreID]> THRESHOLD_HAT) 
                gamepad_buttons[coreID] |= gamepad_down;
        break;

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


        default : 
            // do nothing
        break;
    }

}
