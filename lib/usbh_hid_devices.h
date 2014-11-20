#pragma once

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usbh_hid_core.h"

// external interface of devices
#include "bitbox.h"


extern HID_cb_TypeDef HID_KEYBRD_cb;
extern HID_cb_TypeDef HID_GAMEPAD_cb;
extern HID_cb_TypeDef HID_MOUSE_cb;

typedef struct {
	// FIXME ! more general for other kind of devices, define _type_ of device , do not assume gamepad?
	// discriminator + union ..

    uint16_t vid, pid, pid2, pid3; // applicable vid & pid for the device. pid2,3 : alternate pids
    // non zero VID means valid descriptor

    // config

    /* hat type : 
     *   0 : None, use analog (assumes analog type is not 0 ! )
     *   1 : 8-way hat switch values 0-7 0-270 degrees (or 0-315) + -1 rest
     *   2 : LDRU individual bits
     *   3 : 4-way hat switch values 0-3 + -1 rest
     */
    unsigned int dpad_type:3;

    /* analog type : 
     *   0 : None, use dpad, XY values not used
     *   1 : 2x8 bit axis, 0 default (int8)
     *   2 : 2x8 bit axis, 127 default (uint8)
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
    // could be simplified with u16 button_start_bit + u8 offsets 

    uint16_t reserved_data;   // 32 bytes is a nicely aligned data size
} USB_Gamepad_descriptor;

int USBH_ParseHIDReportDesc(USB_Gamepad_descriptor *desc, uint8_t *data);

extern USB_Gamepad_descriptor gamepad_parsed_descriptor[2]; // in usbh_hid_gamepad.c
