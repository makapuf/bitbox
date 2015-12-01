/* This is bitbox emulated configuration options.
 */

#include <stdint.h>

#define VGA_FPS 60

#if defined(VGAMODE_800) || defined (VGAMODE_800_OVERCLOCK)

// 800 600 non VESA on lightly O/C core (180MHz) - ~ 30kHz, 56fps

#define VGA_H_PIXELS 800 
#define VGA_V_PIXELS 600

#elif defined(VGAMODE_400)
// 400x300 based on 800x600 + skipline / non VESA on lightly O/C core (180MHz) - ~ 30kHz, 56fps

#define VGA_SKIPLINE
#define VGA_H_PIXELS 400 
#define VGA_V_PIXELS 300

#elif defined(VGAMODE_320)

// 320 240 non completely VESA on non O/C core (168MHz) - 30kHz, 60fps
#define VGA_SKIPLINE
#define VGA_H_PIXELS 320 
#define VGA_V_PIXELS 240

#else // default 640x480

#define VGA_H_PIXELS 640 
#define VGA_V_PIXELS 480

#endif

