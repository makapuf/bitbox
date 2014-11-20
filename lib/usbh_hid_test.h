// sample usb headers to check & test the hid parser 
// see usb_parse.c 

#include <stdint.h>

char *global_types[] = {
    "Usage Page","Logical Minimum","Logical Maximum","Physical Minimum",
    "Physical Maximum","Unit Exponent","Unit","Report Size","Report ID",
    "Report Count","Push","Pop",
};

char *local_types[]  = {
	"Usage","Usage Minimum","Usage Maximum","Designator Index",
	"Designator Minimum","Designator Maximum","* Reserved",
	"String Index","String Minimum","String Maximum","Delimiter"
};

typedef struct {
    uint16_t vid, pid, pid2, pid3; // applicable vid & pid for the device. pid2,3 : alternate pids
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

    uint16_t reserved_data;   // 32 bytes is a nicely aligned data size
} USB_Gamepad_descriptor;


uint8_t *sample_reports[]={
	(uint8_t[]){
	 0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	 0x09, 0x05, // USAGE (Game Pad)
	 0xa1, 0x01, // COLLECTION (Application)
	 0x05, 0x02, // USAGE_PAGE (Simulation Controls)
	 0x09, 0xbb, // USAGE (Throttle)
	 0x15, 0x00, // LOGICAL_MINIMUM (0)
	 0x25, 0x1f, // LOGICAL_MAXIMUM (31)
	 0x75, 0x08, // REPORT_SIZE (8)
	 0x95, 0x01, // REPORT_COUNT (1)
	 0x81, 0x02, // INPUT (Data,Var,Abs)
	 0x05, 0x02, // USAGE_PAGE (Simulation Controls)
	 0x09, 0xbb, // USAGE (Throttle)
	 0x15, 0x00, // LOGICAL_MINIMUM (0)
	 0x25, 0x1f, // LOGICAL_MAXIMUM (31)
	 0x75, 0x08, // REPORT_SIZE (8)
	 0x95, 0x01, // REPORT_COUNT (1)
	 0x81, 0x02, // INPUT (Data,Var,Abs)
	 0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	 0xa1, 0x00, // COLLECTION (Physical)
	 0x09, 0x30, // USAGE (X)
	 0x09, 0x31, // USAGE (Y)
	 0x09, 0x32, // USAGE (Z)
	 0x09, 0x33, // USAGE (Rx)
	 0x15, 0x81, // LOGICAL_MINIMUM (-127)
	 0x25, 0x7f, // LOGICAL_MAXIMUM (127)
	 0x75, 0x08, // REPORT_SIZE (8)
	 0x95, 0x04, // REPORT_COUNT (4)
	 0x81, 0x02, // INPUT (Data,Var,Abs)
	 0x05, 0x09, // USAGE_PAGE (Button)
	 0x19, 0x01, // USAGE_MINIMUM (Button 1)
	 0x29, 0x10, // USAGE_MAXIMUM (Button 16)
	 0x15, 0x00, // LOGICAL_MINIMUM (0)
	 0x25, 0x01, // LOGICAL_MAXIMUM (1)
	 0x75, 0x01, // REPORT_SIZE (1)
	 0x95, 0x10, // REPORT_COUNT (16)
	 0x81, 0x02, // INPUT (Data,Var,Abs)
	 0xc0, // END_COLLECTION
	 0xc0, // END_COLLECTION
	},(uint8_t[]){ // logitech 
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x04,        // Usage (Joystick)
    0xA1, 0x01,        // Collection (Application)
    0xA1, 0x02,        //   Collection (Logical)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x35, 0x00,        //     Physical Minimum (0)
    0x46, 0xFF, 0x00,  //     Physical Maximum (255)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x0A,        //     Report Count (10)
    0x25, 0x01,        //     Logical Maximum (1)
    0x45, 0x01,        //     Physical Maximum (1)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x0A,        //     Usage Maximum (0x0A)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x06,        //     Report Count (6)
    0x25, 0x01,        //     Logical Maximum (1)
    0x45, 0x01,        //     Physical Maximum (1)
    0x09, 0x01,        //     Usage (0x01)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0,              // End Collection
	},(uint8_t[]){ // sixaxis
        0x05, 0x01,         /*  Usage Page (Desktop),               */
		0x09, 0x04,         /*  Usage (Joystik),                    */
		0xA1, 0x01,         /*  Collection (Application),           */
		0xA1, 0x02,         /*      Collection (Logical),           */
		0x85, 0x01,         /*          Report ID (1),              */
		0x75, 0x08,         /*          Report Size (8),            */
		0x95, 0x01,         /*          Report Count (1),           */
		0x15, 0x00,         /*          Logical Minimum (0),        */
		0x26, 0xFF, 0x00,   /*          Logical Maximum (255),      */
		0x81, 0x03,         /*          Input (Constant, Variable), */
		0x75, 0x01,         /*          Report Size (1),            */
		0x95, 0x13,         /*          Report Count (19),          */
		0x15, 0x00,         /*          Logical Minimum (0),        */
		0x25, 0x01,         /*          Logical Maximum (1),        */
		0x35, 0x00,         /*          Physical Minimum (0),       */
		0x45, 0x01,         /*          Physical Maximum (1),       */

		0x05, 0x09,         /*          Usage Page (Button),        */
		0x19, 0x01,         /*          Usage Minimum (01h),        */
		0x29, 0x13,         /*          Usage Maximum (13h),        */
		0x81, 0x02,         /*          Input (Variable),           */
		0x75, 0x01,         /*          Report Size (1),            */
		0x95, 0x0D,         /*          Report Count (13),          */
		0x06, 0x00, 0xFF,   /*          Usage Page (FF00h),         */
		0x81, 0x03,         /*          Input (Constant, Variable), */
		0x15, 0x00,         /*          Logical Minimum (0),        */
		0x26, 0xFF, 0x00,   /*          Logical Maximum (255),      */

		0x05, 0x01,         /*          Usage Page (Desktop),       */
		0x09, 0x01,         /*          Usage (Pointer),            */
		0xA1, 0x00,         /*          Collection (Physical),      */
		0x75, 0x08,         /*              Report Size (8),        */
		0x95, 0x04,         /*              Report Count (4),       */
		0x35, 0x00,         /*              Physical Minimum (0),   */
		0x46, 0xFF, 0x00,   /*              Physical Maximum (255), */
		0x09, 0x30,         /*              Usage (X),              */
		0x09, 0x31,         /*              Usage (Y),              */
		0x09, 0x32,         /*              Usage (Z),              */
		0x09, 0x35,         /*              Usage (Rz),             */
		0x81, 0x02,         /*              Input (Variable),       */
		0xC0,               /*          End Collection,             */

		0x05, 0x01,         /*          Usage Page (Desktop),       */
		0x95, 0x13,         /*          Report Count (19),          */
		0x09, 0x01,         /*          Usage (Pointer),            */
		0x81, 0x02,         /*          Input (Variable),           */
		0x95, 0x0C,         /*          Report Count (12),          */
		0x81, 0x01,         /*          Input (Constant),           */
		0x75, 0x10,         /*          Report Size (16),           */
		0x95, 0x04,         /*          Report Count (4),           */
		0x26, 0xFF, 0x03,   /*          Logical Maximum (1023),     */
		0x46, 0xFF, 0x03,   /*          Physical Maximum (1023),    */
		0x09, 0x01,         /*          Usage (Pointer),            */
		0x81, 0x02,         /*          Input (Variable),           */
		0xC0,               /*      End Collection,                 */
		0xA1, 0x02,         /*      Collection (Logical),           */
		0x85, 0x02,         /*          Report ID (2),              */
		0x75, 0x08,         /*          Report Size (8),            */
		0x95, 0x30,         /*          Report Count (48),          */
		0x09, 0x01,         /*          Usage (Pointer),            */
		0xB1, 0x02,         /*          Feature (Variable),         */
		0xC0,               /*      End Collection,                 */
		0xA1, 0x02,         /*      Collection (Logical),           */
		0x85, 0xEE,         /*          Report ID (238),            */
		0x75, 0x08,         /*          Report Size (8),            */
		0x95, 0x30,         /*          Report Count (48),          */
		0x09, 0x01,         /*          Usage (Pointer),            */
		0xB1, 0x02,         /*          Feature (Variable),         */
		0xC0,               /*      End Collection,                 */
		0xA1, 0x02,         /*      Collection (Logical),           */
		0x85, 0xEF,         /*          Report ID (239),            */
		0x75, 0x08,         /*          Report Size (8),            */
		0x95, 0x30,         /*          Report Count (48),          */
		0x09, 0x01,         /*          Usage (Pointer),            */
		0xB1, 0x02,         /*          Feature (Variable),         */
		0xC0,               /*      End Collection,                 */
		0xC0,                /*  End Collection                      */
	},(uint8_t[])	{
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x15, 0x00,                    // LOGICAL_MINIMUM (0)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x02,                    //   USAGE_PAGE (Simulation Controls)
    0x09, 0xbb,                    //   USAGE (Throttle)
    0x15, 0x81,                    //   LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //   LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0x09, 0x39,                    //   USAGE (Hat switch)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x03,                    //   LOGICAL_MAXIMUM (3)
    0x35, 0x00,                    //   PHYSICAL_MINIMUM (0)
    0x46, 0x0e, 0x01,              //   PHYSICAL_MAXIMUM (270)
    0x65, 0x14,                    //   UNIT (Eng Rot:Angular Pos)
    0x75, 0x04,                    //   REPORT_SIZE (4)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x04,                    //   USAGE_MAXIMUM (Button 4)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
0x95, 0x04,                    //   REPORT_COUNT (4)
0x55, 0x00,                    //   UNIT_EXPONENT (0)
0x65, 0x00,                    //   UNIT (None)
0x81, 0x02,                    //   INPUT (Data,Var,Abs)
0xc0,                           // END_COLLECTION
	}, (uint8_t[])
	{ // thrustmaster analog 3
		  0x05, 0x01, 0x09, 0x05, 0xa1, 0x01, 0x05, 0x01, 0x09, 0x01, 0xa1, 0x00,
		  0x05, 0x09, 0x19, 0x01, 0x29, 0x0c, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01,
		  0x95, 0x0c, 0x81, 0x02, 0x75, 0x08, 0x95, 0x01, 0x81, 0x01, 0x05, 0x01,
		  0x09, 0x39, 0x25, 0x07, 0x35, 0x00, 0x46, 0x0e, 0x01, 0x66, 0x40, 0x00,
		  0x75, 0x04, 0x81, 0x42, 0x09, 0x30, 0x09, 0x31, 0x15, 0x80, 0x25, 0x7f,
		  0x46, 0xff, 0x00, 0x66, 0x00, 0x00, 0x75, 0x08, 0x95, 0x02, 0x81, 0x02,
		  0x09, 0x35, 0x95, 0x01, 0x81, 0x02, 0x09, 0x36, 0x16, 0x00, 0x00, 0x26,
		  0xff, 0x00, 0x81, 0x02, 0x09, 0xbb, 0x15, 0x00, 0x26, 0xff, 0x00, 0x35,
		  0x00, 0x46, 0xff, 0x00, 0x75, 0x08, 0x95, 0x04, 0x91, 0x02, 0xc0, 0xc0

	}
};
