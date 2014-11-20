#pragma once

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usbh_hid_core.h"

// external interface of devices
#include "bitbox.h"


extern HID_cb_TypeDef HID_KEYBRD_cb;
extern HID_cb_TypeDef HID_GAMEPAD_cb;
extern HID_cb_TypeDef HID_MOUSE_cb;

