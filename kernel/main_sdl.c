#include <stdlib.h>

#include <SDL/SDL.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

// emulated interfaces
#define draw_buffer __draw_buffer // prevents defining draw buffers to pixel_t 
#include "bitbox.h"
#undef draw_buffer

#define DIR NIX_DIR // prevents name clashes with datfs DIR
#include <dirent.h>
#undef DIR

#include "fatfs/ff.h"

// ticks in ms
#define TICK_INTERVAL 1000/60
#define USER_BUTTON_KEY SDLK_F12

#define KBR_MAX_NBR_PRESSED 6
#define RESYNC_TIME_MS 2000 // dont try to sync back if delay if more than 

#define WM_TITLE_LED_ON  "Bitbox emulator (*)"
#define WM_TITLE_LED_OFF "Bitbox emulator"

#define THRESHOLD_HAT 30

/*
   TODO

 handle FULLSCREEN (alt-enter) as keyboard handles
 handling other events (plugged, ...)

 game recordings

*/


// emulated screen size, possibly displayed larger
int screen_width; 
int screen_height;

#define LINE_BUFFER 1024
#ifndef LINE_MARGIN
#define LINE_MARGIN 32 
#endif

// options
static int slow; // parameter : run slower ?
static int fullscreen; // shall run fullscreen
static int quiet=1; // quiet by default now
static int scale=VGA_V_PIXELS<400 ? 2 : 1; // scale display by this in pixels

// Video
SDL_Surface* screen;
uint16_t mybuffer1[LINE_BUFFER];
uint16_t mybuffer2[LINE_BUFFER];
uint16_t *draw_buffer = mybuffer1+LINE_MARGIN; // volatile ?

volatile uint16_t gamepad_buttons[2];
uint16_t kbd_gamepad_buttons;
uint16_t sdl_gamepad_buttons[2]; // real gamepad read from SDL
bool sdl_gamepad_from_axis[2]; // emulate buttons from gamepad ?

uint32_t vga_line;
volatile uint32_t vga_frame;
#ifdef VGA_SKIPLINE
volatile int vga_odd;
#endif

// IO
volatile int8_t mouse_x, mouse_y;
volatile uint8_t mouse_buttons;
 
int user_button=0;

// joystick handling.
static const int gamepad_max_buttons = 12;
static const int gamepad_max_pads = 2;

volatile int8_t gamepad_x[2], gamepad_y[2]; // analog pad values

volatile uint8_t keyboard_mod[2]; // LCtrl =1, LShift=2, LAlt=4, LWin - Rctrl, ...
volatile uint8_t keyboard_key[2][KBR_MAX_NBR_PRESSED]; // using raw USB key codes



#if VGA_MODE != NONE
extern uint16_t palette_flash[256];

void __attribute__((weak)) graph_vsync() {} // default empty


void expand_buffer ( void )
{
    if (vga_odd) {
        // expand in place buffer from 8bits RRRGGBBL to 15bits RRRrrGGLggBBLbb
        // cost is ~ 5 cycles per pixel. not accelerated by putting palette in CCMRAM
        const uint32_t * restrict src = (uint32_t*)&draw_buffer[VGA_H_PIXELS/2-2];
        uint32_t * restrict dst=(uint32_t*)&draw_buffer[VGA_H_PIXELS-2];
        for (int i=0;i<VGA_H_PIXELS/4;i++) {
            uint32_t pix=*src--; // read 4 src pixels
            *dst-- = palette_flash[pix>>24]<<16         | palette_flash[(pix>>16) &0xff]; // write 2 pixels
            *dst-- = palette_flash[(pix>>8) & 0xff]<<16 | palette_flash[pix &0xff]; // write 2 pixels
        }
    }
}


/* naive pixel conversion
from 16bit bitbox pixel 0RRRRRGGGGGBBBBB
to 16bit color (565) RRRRRGGGGG0BBBBB
*/
static inline uint16_t pixelconv(uint16_t pixel)
{
    return (pixel & (uint16_t)(~0x1f))<<1 | (pixel & 0x1f);
}

// 0RRRRRGGGGGBBBBB to RGB032 rrrrr000 ggggg000 bbbbb000 00000000
static inline uint32_t pixelconv32(uint16_t pixel)
{
    return ((pixel & (0x1f<<10))<<9 | (pixel & (0x1f<<5))<<6 | (pixel & 0x1f)<<3);
}

static void __attribute__ ((optimize("-O3"))) refresh_screen (SDL_Surface *scr)
// uses global line + vga_odd, scale two times
{

    uint32_t * restrict dst = (uint32_t*)scr->pixels; // will render 2 pixels at a time horizontally

    draw_buffer = mybuffer1+LINE_MARGIN; // currently 16bit data

    for (vga_line=0;vga_line<screen_height;vga_line++) {
        #ifdef VGA_SKIPLINE
            vga_odd=0;
            graph_line(); // using line, updating draw_buffer ...
            #if VGA_BPP==8
            expand_buffer();
            #endif 
            vga_odd=1; 
            graph_line(); //  a second time for SKIPLINE modes
            #if VGA_BPP==8
            expand_buffer();
            #endif 
        #else 
            graph_line(); 
            #if VGA_BPP==8
            expand_buffer();
            #endif 
        #endif

        // copy to screen at this position
        uint16_t *restrict src = (uint16_t*) draw_buffer;
        switch (scale) {
            case 1 : 
                // copy to screen at this position (cheating)
                for (int i=0;i<screen_width;i++)
                    *dst++= pixelconv32(*src++);
                break;

            case 2 : 
                for (int i=0;i<screen_width;i++, dst+=2) {
                    uint32_t pix = pixelconv32(*src++);
                    *dst = pix; // blit line
                    *(dst+1) = pix; // blit line
                    
                    *(dst+scr->pitch/sizeof(uint32_t))=pix; // also next line
                    *(dst+scr->pitch/sizeof(uint32_t)+1)=pix; // also next line
                    
                }
                dst += scr->pitch/sizeof(uint32_t); // we already drew the line after, skip it
                break;
        } 


        // swap lines buffers to simulate double line buffering
        draw_buffer = ( draw_buffer == &mybuffer1[LINE_MARGIN] ) ? &mybuffer2[LINE_MARGIN] : &mybuffer1[LINE_MARGIN];

    }
    for (;vga_line<screen_height+VGA_V_SYNC;vga_line++) {
        #ifdef VGA_SKIPLINE
            vga_odd=0;
            graph_vsync(); // using line, updating draw_buffer ...
            vga_odd=1; 
            graph_vsync(); //  a second time for SKIPLINE modes
        #else 
            graph_vsync(); // once
        #endif
    }
}
#else
#warning VGA_MODE SET TO NONE
#endif


#ifndef NO_AUDIO
static uint16_t bitbox_sound_buffer[BITBOX_SNDBUF_LEN];
int bitbox_sound_idx = BITBOX_SNDBUF_LEN;

static void __attribute__ ((optimize("-O3"))) mixaudio(void * userdata, uint8_t * stream, int len)
// this callback is called each time we need to fill the buffer
{
    int i=0;
    uint16_t *dst = (uint16_t *)stream;
    len /= 2;
    while (i < len) {
        if (bitbox_sound_idx == BITBOX_SNDBUF_LEN) {
            bitbox_sound_idx = 0;
            game_snd_buffer(bitbox_sound_buffer, BITBOX_SNDBUF_LEN);
        }
        while (bitbox_sound_idx < BITBOX_SNDBUF_LEN && i < len)
            dst[i++] = bitbox_sound_buffer[bitbox_sound_idx++];
    }
#ifdef __HAIKU__
	// On Haiku, U8 audio format is broken so we convert to signed
	for (i = 0; i < len; i++)
		stream[i] -= 128;
#endif
}

void audio_init(void)
{
    SDL_AudioSpec desired;

    desired.freq = BITBOX_SAMPLERATE;
#ifdef __HAIKU__
    desired.format = AUDIO_S8;
#else
    desired.format = AUDIO_U8;
#endif
    desired.channels = 2;

    /* The audio buffers holds at least one vga_frame worth samples */
    desired.samples = BITBOX_SNDBUF_LEN;

    /* Setup callback and "user data" parameter passed to it (we don't use it) */
    desired.callback = &mixaudio;
    desired.userdata = NULL;

    if (!quiet) {
       printf("Bitbox sndbuflen : %d\n",BITBOX_SNDBUF_LEN);
       printf("Audio parameters wanted (before): format=%d, %d channels, fs=%d, %d samples.\n",
            desired.format , desired.channels, desired.freq, desired.samples);
    }

    if (SDL_OpenAudio(&desired, NULL) != 0) {
        printf("Error in opening audio peripheral: %s\n", SDL_GetError());
        return ; // return anyways even with no sound
    }

    if (!quiet)
        printf("Audio parameters obtained (after): format=%d, %d channels, fs=%d, %d samples.\n",
        desired.format , desired.channels, desired.freq, desired.samples);

}

// default empty implementation
__attribute__((weak)) void game_snd_buffer(uint16_t *buffer, int len)  {}
#endif

void set_mode(int width, int height)
{
    screen_width = width;
    screen_height = height;
    screen = SDL_SetVideoMode(width*scale,height*scale, 32, SDL_HWSURFACE|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0));
    if ( !screen )
    {
        printf("%s\n",SDL_GetError());
        die(-1,0);
    }

    if (!quiet) {
        printf("%d bpp, flags:%x pitch %d\n", screen->format->BitsPerPixel, screen->flags, screen->pitch/2);
        printf("Screen is now %dx%d with a scale of %d\n",screen_width,screen_height,scale);
    }

}

static void joy_init()
// by David Lau
{
    int i;
    int joy_count;

    /* Initilize the Joystick, and disable all later joystick code if an error occured */

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK))
        return;

    joy_count = SDL_NumJoysticks();

    /* Open all joysticks, assignement is in opening order */
    for (i = 0; i < joy_count; i++)
    {
        SDL_Joystick *joy = SDL_JoystickOpen(i);

        if (!quiet) 
            printf("found Joystick %d : %d axis, %d buttons, %d hats (emu=%d)\n",
                i,
                SDL_JoystickNumAxes(joy),
                SDL_JoystickNumButtons(joy),
                SDL_JoystickNumHats(joy), 
                SDL_JoystickNumHats(joy)==0 
                );

        if (i<2) {
            sdl_gamepad_from_axis[i]=SDL_JoystickNumHats(joy)==0;
        }
    }

    /* make sure that Joystick sdl_event polling is a go */
    SDL_JoystickEventState(SDL_ENABLE);
}


void instructions ()
{
    printf("Invoke game with those options : \n");
    printf("  --fullscreen to run fullscreen\n");
    printf("  --scale2x to scale display 2x (also --scale1x)\n");
    printf("  --slow to run game very slowly (for debug)\n");
    printf("  --verbose show helpscreen and various messages\n");
    printf("\n");
    printf("Use Joystick, Mouse or keyboard.");
    printf("Bitbox user Button is emulated by the F12 key.\n");
    printf("       -------\n");
    printf("Some games emulate Gamepad with the following keyboard keys :\n");
    printf("    Space (Select),   Enter (Start),   Arrows (D-pad)\n");
    printf("    D (A button),     F (B button),    E (X button), R (Y button),\n");
    printf("    Left/Right CTRL (L/R shoulders),   ESC (quit)\n");
    printf("       -------\n");
}



// reverse mapping to USB BootP

#ifdef __HAIKU__
uint8_t key_trans[256] = {
	[0x01]=0,41, 58,59,60,61, 62,63,64,65, 66,67,68,69,  70,71,72,

	[0x11]=53,30,31,32,33,34,35,36,37,38,39,45,46,42,    73,74,75,  83,84,85,86,
	[0x26]=43, 20,26, 8,21,23,28,24,12,18,19,47,48,49,   76,77,78,  95,96,97,87,
	[0x3b]=57,   4,22, 7, 9,10,11,13,14,15,51,52,40,                92,93,94,
	[0x4b]=225,  29,27, 6,25, 5,17,16,54,55,56,229,         82,     89,90,91,88,
	[0x5c]=224,226,           44             ,230,228,   80,81,79,  98,   99,
	        227,                              231,118
};
#else
uint8_t key_trans[256] = { // scan_code -> USB BootP code
    [0x6F]=0x52, // up
    [0x74]=0x51, // down
    [0x71]=0x50, // left
    [0x72]=0x4F, // right
    [0x41]=0x2C, // space
    [0x09]=0x29, // ESC -- needs to define DISABLE_ESC_EXIT in makefile to avoid escaping emulator !
    [0x17]=0x2B, // TAB
    [0x16]=42, // backspace on mine... (lowagner)
    [0x77]=76, // delete
    [0x76]=73, // insert
    [0x7f]=0x48, // pause

    [0x0a]=30,31,32,33,34,35,36,37,38,39,45,46, // 1234567890-=
    [0x18]=20,26, 8,21,23,28,  24,  12,  18,  19,  // qwertyuiop
    [0x22]=47,48, // []
    [0x26]= 4,22, 7, 9,10,11,13,14,15,16, // asdfghjklm
    [0x30]=52,53, // ' and `
    [0x33]=49,  //  backslash
    [0x34]=29,27, 6,25, 5,17,54,55,56, // zxcvbnm,./

    [0x6e]=74, // home
    [0x73]=77, // end
    [0x70]=75, // pgup
    [0x75]=78, // pgdn
    [0x5a]=98,99, // 0. on number pad
    [0x57]=89,90,91, // 1 through 3
    [0x53]=92,93,94, // 4 through 6
    [0x4f]=95,96,97, // 7 through 9 on number pad

    [0x32]=225, // left shift
    [0x3e]=229, // right shift
    [0x40]=226, // L alt
    [0x6c]=230, // R alt
    [0x25]=0xe0, // L CTRL
    [0x69]=0xe4, // R CTRL
    [0x85]=0xe0, // L cmd (mac)
    [0x86]=0xe4, // R cmd (mac)
    [0x24]=0x28, // Enter

};
#endif

// this is a copy of the same function in usbh_hid_keybd
void kbd_emulate_gamepad (void)
{
    // kbd code for each gamepad buttons 
    static const uint8_t kbd_gamepad[] = {
        0x07, 0x09, 0x08, 0x15, 0xe0, 0xe4, 0x2c, 0x28, 0x52, 0x51, 0x50, 0x4f
    };

    kbd_gamepad_buttons = 0;
    for (int i=0;i<sizeof(kbd_gamepad);i++) {
        if (memchr((char *)keyboard_key[0],kbd_gamepad[i],KBR_MAX_NBR_PRESSED))
            kbd_gamepad_buttons |= (1<<i);
    }

    // special : mods
    if (keyboard_mod[0] & 1) 
        kbd_gamepad_buttons |= gamepad_L;
    if (keyboard_mod[0] & 16) 
        kbd_gamepad_buttons |= gamepad_R;

}

// emualte hat from axis
void emulate_joy_hat(void)
{
    for (int pad=0;pad<gamepad_max_pads;pad++)
        if (sdl_gamepad_from_axis[pad]) {
            // clear movement bits
            sdl_gamepad_buttons[pad] &= ~(gamepad_up | gamepad_down | gamepad_left | gamepad_right);

            // set according to axis
            if (gamepad_x[pad]<-THRESHOLD_HAT)
                sdl_gamepad_buttons[pad] |= gamepad_left;
            if (gamepad_x[pad]> THRESHOLD_HAT) 
                sdl_gamepad_buttons[pad] |= gamepad_right;

            if (gamepad_y[pad]<-THRESHOLD_HAT) 
                    sdl_gamepad_buttons[pad] |= gamepad_up;
            if (gamepad_y[pad]> THRESHOLD_HAT) 
                    sdl_gamepad_buttons[pad] |= gamepad_down;

        }

}


static bool handle_events()
{
    SDL_Event sdl_event;
    uint8_t key;
    //mouse_x = mouse_y=0; // not moved this frame 

    while (SDL_PollEvent(&sdl_event))
    {
        // check for messages
        switch (sdl_event.type)
        {
        // exit if the window is closed
        case SDL_QUIT:
            return true;
            break;

        // check for keypresses
        case SDL_KEYDOWN:
            #ifndef DISABLE_ESC_EXIT
            if (sdl_event.key.keysym.sym == SDLK_ESCAPE)
                return true; // quit now
            #endif

            /* note that this event WILL be propagated so on emulator
            you'll see both button and keyboard. It's ot really a problem since
            programs rarely use the button and the keyboard */
            if (sdl_event.key.keysym.sym == USER_BUTTON_KEY)
                user_button=1;

            // now create the keyboard event
            key = key_trans[sdl_event.key.keysym.scancode];
            // mod key ? 
            switch (key) {
                case 0xe0: // lctrl
                    keyboard_mod[0] |= 1; 
                    break;
                case 225: // lshift
                    keyboard_mod[0] |= 2;
                    break;
                case 226: // lalt
                    keyboard_mod[0] |= 4;
                    break;

                case 0xe4: // rctrl
                    keyboard_mod[0] |= 16; 
                    break;
                case 229: // rshift
                    keyboard_mod[0] |= 32;
                    break;
                case 230: // ralt
                    keyboard_mod[0] |= 64;
                    break;

                default : // set it 
                    for (int i=0;i<KBR_MAX_NBR_PRESSED;i++) {
                        if (keyboard_key[0][i]==0) {
                            keyboard_key[0][i]=key;
                            break;
                        }
                    }
            }

            //message("keydown %x\n",sdl_event.key.keysym.scancode);
            break;

        case SDL_KEYUP:

            if (sdl_event.key.keysym.sym == USER_BUTTON_KEY)
                user_button=0;

            key = key_trans[sdl_event.key.keysym.scancode];
            // mod key ? 
            switch (key) {
                case 0xe0: // lctrl
                    keyboard_mod[0] &= ~1; 
                    break;
                case 225: // lshift
                    keyboard_mod[0] &= ~2;
                    break;
                case 226: // lalt
                    keyboard_mod[0] &= ~4;
                    break;

                case 0xe4: // rctrl
                    keyboard_mod[0] &= ~16; 
                    break;
                case 229: // rshift
                    keyboard_mod[0] &= ~32;
                    break;
                case 230: // ralt
                    keyboard_mod[0] &= ~64;
                    break;

                default : 
                    // other one : find it & release it 
                    for (int i=0;i<KBR_MAX_NBR_PRESSED;i++) {
                        if (key==keyboard_key[0][i]) {
                            keyboard_key[0][i]=0;
                            break;
                        }
                    }
            }

            // message("keyup %x\n",sdl_event.key.keysym.scancode );
            break;

        // joypads
        case SDL_JOYAXISMOTION: // analog position
            switch (sdl_event.jaxis.axis) {
                case 0: /* X axis */
                    gamepad_x[sdl_event.jbutton.which]=sdl_event.jaxis.value>>8;
                    break;
                case 1: /* Y axis*/
                    gamepad_y[sdl_event.jbutton.which]=sdl_event.jaxis.value>>8;
                    break;
            }
            break;

        case SDL_JOYBUTTONUP: // buttons
            if (sdl_event.jbutton.button>=gamepad_max_buttons || sdl_event.jbutton.which>=gamepad_max_pads)
                break;
            sdl_gamepad_buttons[sdl_event.jbutton.which] &= ~(1<<sdl_event.jbutton.button);
            break;

        case SDL_JOYBUTTONDOWN:
            if (sdl_event.jbutton.button>=gamepad_max_buttons || sdl_event.jbutton.which>=gamepad_max_pads)
                break;
            sdl_gamepad_buttons[sdl_event.jbutton.which] |= 1<<sdl_event.jbutton.button;
            break;

        case SDL_JOYHATMOTION: // HAT
            if (sdl_event.jbutton.which>=gamepad_max_pads)
                break;
            sdl_gamepad_buttons[sdl_event.jbutton.which] &= ~(gamepad_up|gamepad_down|gamepad_left|gamepad_right);
            if (sdl_event.jhat.value & SDL_HAT_UP)      sdl_gamepad_buttons[sdl_event.jbutton.which] |= gamepad_up;
            if (sdl_event.jhat.value & SDL_HAT_DOWN)    sdl_gamepad_buttons[sdl_event.jbutton.which] |= gamepad_down;
            if (sdl_event.jhat.value & SDL_HAT_LEFT)    sdl_gamepad_buttons[sdl_event.jbutton.which] |= gamepad_left;
            if (sdl_event.jhat.value & SDL_HAT_RIGHT)   sdl_gamepad_buttons[sdl_event.jbutton.which] |= gamepad_right;
            break;

        // mouse
        case SDL_MOUSEBUTTONDOWN:
            mouse_buttons |= 1<<(sdl_event.button.button-1);
            break;
        case SDL_MOUSEBUTTONUP:
            mouse_buttons &= ~1<<(sdl_event.button.button-1);
            break;
        case SDL_MOUSEMOTION :
            mouse_x += sdl_event.motion.xrel;
            mouse_y += sdl_event.motion.yrel;
            break;

        } // end switch
    } // end of message processing

    return false; // don't exit  now
}

// -------------------------------------------------
// limited fatfs-related functions.
// XXX add non readonly features
FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt)
{
    return FR_OK;
}

FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode)
{
    char *mode_host=0;

    // XXX quite buggy ...
    if (mode & FA_OPEN_ALWAYS) {
        if (!access(path, F_OK)) // 0 if OK
            mode_host = "r+";
        else
            mode_host = "w+";

    } else switch (mode) {
        // Not a very good approximation, should rewrite to handle properly
        case FA_READ | FA_OPEN_EXISTING : mode_host="r"; break;
        case FA_READ | FA_WRITE | FA_OPEN_EXISTING : mode_host="r+"; break;
        case FA_WRITE | FA_OPEN_EXISTING : mode_host="r+"; break; // faked

        case FA_WRITE | FA_CREATE_NEW : mode_host="wx"; break;
        case FA_READ | FA_WRITE | FA_CREATE_NEW : mode_host="wx+"; break;

        case FA_READ | FA_WRITE | FA_CREATE_ALWAYS : mode_host="w+"; break;
        case FA_WRITE | FA_CREATE_ALWAYS : mode_host="w"; break;

        default :
            return FR_DISK_ERR;
    }

    fp->fs = (FATFS*) fopen ((const char*)path,mode_host); // now ignores mode.
    return fp->fs ? FR_OK : FR_DISK_ERR; // XXX duh.
}

FRESULT f_close (FIL* fp)
{
    int res = fclose( (FILE*) fp->fs);
    fp->fs=NULL;
    return res?FR_DISK_ERR:FR_OK; // FIXME handle reasons ?
}

FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br)
{
    *br = fread ( buff, 1,btr, (FILE *)fp->fs);
    return FR_OK; // XXX handle ferror
}

FRESULT f_write (FIL* fp, const void* buff, UINT btr, UINT* br)
{
    *br = fwrite ( buff,1, btr, (FILE *)fp->fs);
    return FR_OK; // XXX handle ferror
}


FRESULT f_lseek (FIL* fp, DWORD ofs)
{
    int res = fseek ( (FILE *)fp->fs, ofs, SEEK_SET);
    return res ? FR_DISK_ERR : FR_OK; // always from start
}

/* Change current directory */
FRESULT f_chdir (const char* path)
{
    int res = chdir(path);
    return res ? FR_DISK_ERR : FR_OK;
}


FRESULT f_opendir ( DIR* dp, const TCHAR* path )
{
    NIX_DIR *res = opendir(path);
    if (res) {
        dp->fs = (FATFS*) res; // hides it in the fs field as a fatfs variable
        dp->dir = (unsigned char *)path;
        return FR_OK;
    } else {
        printf("Error opening directory : %s\n",strerror(errno));
        return FR_DISK_ERR;
    }
}

FRESULT f_readdir ( DIR* dp, FILINFO* fno )
{
    errno=0;
    char buffer[260]; // assumes max path size for FAT32

    struct dirent *de = readdir((NIX_DIR *)dp->fs); 

    if (de) {
        for (int i=0;i<13;i++)
            fno->fname[i]=de->d_name[i];

        fno->fattrib = 0;

        // check attributes of found file
        strncpy(buffer,(char *)dp->dir,260); // BYTE->char
        strcat(buffer,"/");
        strcat(buffer,fno->fname);

        struct stat stbuf;
        stat(buffer,&stbuf);

        if (S_ISDIR(stbuf.st_mode))
             fno->fattrib = AM_DIR;
        return FR_OK;

    } else {
        if (errno) {
            printf("Error reading directory %s : %s\n",dp->dir, strerror(errno)); // not neces an erro, can be end of dir.
            return FR_DISK_ERR;
        } else {
            fno->fname[0]='\0';
            return FR_OK;
        }
    }
}

FRESULT f_closedir (DIR* dp)
{
    if (!closedir((NIX_DIR *)dp->fs)) {
        return FR_OK ;
    } else {
        printf("Error closing directory %s : %s\n",dp->dir, strerror(errno));
        return FR_DISK_ERR;
    }
}

FRESULT f_rename (const char *file_from, const char *file_to)
{
    if (!rename(file_from,file_to) ) {
        return FR_OK;
    } else {
        printf("Error renaming %s to %s : : %s\n",file_from, file_to, strerror(errno));
        return FR_DISK_ERR;
    }
}

// -- misc bitbox functions 

// user button
int button_state() {
    return user_button;
}

// user LED
void set_led(int x) {
    if (!quiet)
        printf("Setting LED to %d\n",x);
    SDL_WM_SetCaption(x?WM_TITLE_LED_ON:WM_TITLE_LED_OFF, "game");
}

void message (const char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    vprintf(fmt, argptr);
    va_end(argptr);
}

void die(int where, int cause)
{
    printf("ERROR : dying doing %d at  %d.\n",cause, where);
    exit(cause);
}


// --- main

static void process_commandline( int argc, char** argv)
{
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i],"--fullscreen"))
            fullscreen = 1;
        else if (!strcmp(argv[i],"--slow"))
            slow = 1;
        else if (!strcmp(argv[i],"--verbose"))
            quiet = 0;
        else if (!strcmp(argv[i],"--scale1x"))
            scale = 1;
        else if (!strcmp(argv[i],"--scale2x"))
            scale = 2;
        else {
            instructions();
            exit(0);
        }
    }

    // display current options
    if (!quiet) {
        printf("Options : %s %s scale : %d\n",fullscreen?"fullscreen":"windowed",slow?"slow":"normal speed",scale);
        instructions();
        printf(" - Starting\n");
    }
}

static void init_all(void)
{
    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 ) {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        exit(1);
    }
    atexit(SDL_Quit); // make sure SDL cleans up before exit


    set_led(0); // off by default
    set_mode(VGA_H_PIXELS,VGA_V_PIXELS); // create a default new window
    SDL_ShowCursor(SDL_DISABLE);    

    #ifndef NO_AUDIO
    audio_init();
    SDL_PauseAudio(0); // now start sound
    #endif

    joy_init();
}

SDL_sem *frame_sem;

void wait_vsync(const unsigned int n)
{
    uint32_t nframe = vga_frame+n;
    while (vga_frame < nframe) {
        // wait other thread to wake me up
        SDL_SemWait(frame_sem);
    }
}

/* this loop handles asynchronous emulation : screen refresh, user inputs.. */
int emu_loop (void *_)
{
    uint32_t now, next_time;
    next_time = SDL_GetTicks();
    bool done=false;

    while (!done) {
        // message processing loop
        done = handle_events();
        kbd_emulate_gamepad();
        emulate_joy_hat();
        gamepad_buttons[0] = kbd_gamepad_buttons|sdl_gamepad_buttons[0];
        gamepad_buttons[1] = sdl_gamepad_buttons[1];



        // 60 Hz loop delay
        now = SDL_GetTicks();        
        if (next_time > now)
            SDL_Delay(next_time - now);

        if (((int)now-(int)next_time) > RESYNC_TIME_MS)  { // if too much delay, resync
            printf("- lost sync : %d, resyncing\n",now-next_time);
            next_time = now;
        } else {
            next_time += slow ? TICK_INTERVAL*10:TICK_INTERVAL;
        }
        
        #if VGA_MODE!=NONE
        refresh_screen(screen);
        #endif
        SDL_Flip(screen);
        vga_frame++;

        // if locked, unlock it
        if (!SDL_SemValue(frame_sem)) {
            SDL_SemPost(frame_sem);
        }
    }
    exit(0);
    return 0;
}

int main ( int argc, char** argv )
{
    process_commandline(argc,argv);
    init_all();

    frame_sem=SDL_CreateSemaphore(0);
    SDL_Thread * thread=SDL_CreateThread(emu_loop,0);  

    bitbox_main();

    SDL_KillThread(thread);
    SDL_DestroySemaphore(frame_sem);

    return 0;
}


