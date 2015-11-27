/* This is bitbox standard kernel configuration options. A standard dev should not modify it, 
 * as this one is used to tune clocks / overclocking, VGA clocks ... 
 * 
 * to override this file in your project, just include a modified kconf.h in your main compile dir. 
 */
#include <stdint.h>


// Modes implied by simple Modes

#if VGA_SIMPLE_MODE == 1 || VGA_SIMPLE_MODE==2
  #define VGAMODE_800
#elif VGA_SIMPLE_MODE==11 
  #define VGAMODE_800_OVERCLOCK
#elif VGA_SIMPLE_MODE==4
  #define VGAMODE_400
#elif VGA_SIMPLE_MODE==5
  #define VGAMODE_320
#else 
  #define VGAMODE_640
#endif 

// basic info about Modes

#if defined (VGAMODE_NONE)

#elif defined(VGAMODE_640_OVERCLOCK)

#define VGA_H_PIXELS 640 
#define VGA_V_PIXELS 480
#define VGA_V_BLANK 45
#define VGA_FPS 60

#elif defined(VGAMODE_800)

#define VGA_H_PIXELS 800 
#define VGA_V_PIXELS 600
#define VGA_V_BLANK 19
#define VGA_FPS 56

#elif defined(VGAMODE_800_OVERCLOCK)


#define VGA_H_PIXELS 800 
#define VGA_V_PIXELS 600
#define VGA_V_BLANK 19
#define VGA_FPS 56

#elif defined(VGAMODE_400)

#define VGA_SKIPLINE
#define VGA_H_PIXELS 400 
#define VGA_V_PIXELS 300
#define VGA_V_BLANK 10
#define VGA_FPS 56

#elif defined(VGAMODE_320)

#define VGA_SKIPLINE
#define VGA_H_PIXELS 320 
#define VGA_V_PIXELS 240
#define VGA_V_FRONTPORCH 10
#define VGA_FPS 60

#else // default one, use 

#define VGA_H_PIXELS 640 
#define VGA_V_PIXELS 480
#define VGA_V_BLANK 20
#define VGA_FPS 60

#endif 

