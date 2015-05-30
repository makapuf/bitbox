#include <stdlib.h>

#include <SDL/SDL.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
 #include <errno.h>

// emulated interfaces
#include "bitbox.h"

#define DIR NIX_DIR // prevents name clashes with datfs DIR
#include <dirent.h>
#undef DIR

#include "fatfs/ff.h"


/*
   TODO
   
 handle SLOW + PAUSE + FULLSCREEN (alt-enter) as keyboard handles 
 handle properly gamepads, second gamepad, 
 handle mouse, 
 keyboard (treat keyboard gamepads as config for quick saves)
 handling other events (plugged, ...)

 really handle set_led (as window title) 

*/ 


// ----------------------------- kernel ----------------------------------
/* The only function of the kernel is
- calling game_init and game_frame
- calling update draw_buffer (which calls object_blitX)

- displaying other buffer updating line, frame
- read user input and update gamepad 

*/

// ticks in ms

int screen_width;
int screen_height;

#define TICK_INTERVAL 1000/60
#define LINE_BUFFER 1024

int slow; // parameter : run slower ?

static uint32_t next_time;

// Video
int fullscreen; // shall run fullscreen
int quiet;

SDL_Surface* screen;
uint16_t mybuffer1[LINE_BUFFER];
uint16_t mybuffer2[LINE_BUFFER];
uint16_t *draw_buffer = mybuffer1; // volatile ?
volatile uint16_t gamepad_buttons[2]; 
uint32_t vga_line; 
volatile uint32_t vga_frame;
volatile int vga_odd;

volatile int data_mouse_x, data_mouse_y;
volatile uint8_t data_mouse_buttons;

int user_button=0; 

// sound
// uint16_t audio_buffer[BITBOX_SNDBUF_LEN]; // stereo, 31khz 1frame

// joystick handling.
static const int gamepad_max_buttons = 12;
static const int gamepad_max_pads = 2;

 // XXX FIXME handle them !
volatile int8_t gamepad_x[2], gamepad_y[2]; // analog pad values

uint32_t time_left(void)
{
    uint32_t now;

    now = SDL_GetTicks();
    if(next_time <= now)
        return 0;
    else
        return next_time - now;
}


/* naive pixel conversion from
0RRRRRGGGGGBBBBB to 16bit color (565)
RRRRRGGGGG0BBBBB
*/
inline uint16_t pixelconv(uint16_t pixel)
{
    return (pixel & (uint16_t)(~0x1f))<<1 | (pixel & 0x1f);
} 


static void refresh_screen(SDL_Surface *scr)
// uses global line + vga_odd
{
    uint16_t *dst = (uint16_t*)scr->pixels;
    
    draw_buffer = mybuffer1;
    graph_frame();

    for (vga_line=0;vga_line<screen_height;vga_line++) {
        vga_odd=0;
        graph_line(); // using line, updating draw_buffer ...
        #ifdef VGA_SKIPLINE
        vga_odd=1; graph_line(); //  a second time for SKIPLINE modes
        #endif 

        // copy to screen at this position (cheating)
        uint16_t *src = (uint16_t*) &draw_buffer[0];//[MARGIN];

        for (int i=0;i<screen_width;i++) 
            *dst++= pixelconv(*src++);

        // swap lines buffers to simulate double line buffering
        draw_buffer = (draw_buffer == &mybuffer1[0] ) ? &mybuffer2[0] : &mybuffer1[0];
    }
}
static void mixaudio(void * userdata, Uint8 * stream, int len)
// this callback is called each time we need to fill the buffer
{
    game_snd_buffer((uint16_t *)stream,len/2); 
}

void audio_init(void)
{
    SDL_AudioSpec desired;

    desired.freq = BITBOX_SAMPLERATE;
    desired.format = AUDIO_U8;
    desired.channels = 2; 

    /* Le tampon audio contiendra at least one vga_frame worth samples */
    desired.samples = BITBOX_SNDBUF_LEN*2; // XXX WHY is it halved ??

    /* Mise en place de la fonction de rappel et des données utilisateur */
    desired.callback = &mixaudio;
    desired.userdata = NULL;
   
    if (!quiet) {
       printf("sndbuflen : %d\n",BITBOX_SNDBUF_LEN);
       printf("Paramètres audio desired (before): format %d,%d canaux, fs=%d, %d samples.\n",
            desired.format , desired.channels, desired.freq, desired.samples);
    }

    //if (SDL_OpenAudio(&desired, &obtained) != 0) {
    if (SDL_OpenAudio(&desired, NULL) != 0) {
        printf("Error in opening audio peripheral: %s\n", SDL_GetError());
        return ; // return anyways even with no sound
    }

    if (!quiet)
        printf("Audio parameters desired (after): format %d,%d canaux, fs=%d, %d samples.\n",
        desired.format , desired.channels, desired.freq, desired.samples);

}

// default empty implementation
__attribute__((weak)) void game_snd_buffer(uint16_t *buffer, int len)  {}

void set_mode(int width, int height)
{
    screen_width = width;
    screen_height = height;
    screen = SDL_SetVideoMode(width,height, 16, SDL_HWSURFACE|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0));
    if ( !screen )
    {
        printf("%s\n",SDL_GetError());
        die(-1,0);
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
    
    if (!joy_count)
        return;

    /* Open all joysticks, assignement is in opening order */
    for (i = 0; i < joy_count; i++)
    {
        SDL_JoystickOpen(i);
    }
    
    /* make sure that Joystick sdl_event polling is a go */
    SDL_JoystickEventState(SDL_ENABLE);
}


int init(void)
{
    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a default new window
    set_mode(VGA_H_PIXELS,VGA_V_PIXELS);
    audio_init();
    joy_init();


    printf("screen is now %dx%d\n",screen_width,screen_height);
    return 0;
}


void instructions ()
{
    printf("Use Joystick, Mouse or keyboard.");
    printf("Bitbox user Button is emulated by the <Pause/Break> key.\n");
    printf("       -------\n");
    printf("Keyboard for some games emulate Gamepad with the following keys \n");
    printf("    Space (Select),   Enter (Start),   Arrows (D-pad)\n");
    printf("    D (A button),     F (B button),    E (X button), R (Y button),\n");
    printf("    Left/Right CTRL (L/R shoulders),   ESC (quit)\n");
    printf("       -------\n");
}


const char * gamepad_names[] = {
    "gamepad_A",
    "gamepad_B",
    "gamepad_X",
    "gamepad_Y",
    "gamepad_L",
    "gamepad_R",
    "gamepad_select",
    "gamepad_start",

    "gamepad_up",    
    "gamepad_down",
    "gamepad_left",
    "gamepad_right",
};

// reverse mapping to USB BootP

uint8_t key_trans[256] = {
    [0x6F]=0x52, // up
    [0x74]=0x51, // down
    [0x71]=0x50, // left
    [0x72]=0x4F, // right
    [0x41]=0x2C, // space
    [0x18]=0x14,0x1a, 0x08, 0x15, 0x17, 0x1c, 0x04, 0x18, 0x0c, 0x12, 0x13, // azertyuiop / qwertyuiop
    [0x26]=0x04,0x16, 0x07, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x2f, // asdfghjklm
    [0x34]=0x1d,0x1c, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38, // zxcvbnm<>/

    [0x25]=0xe0, // L CTRL
    [0x69]=0xe4, // R CTRL
    [0x24]=0x28, // Enter
};

static bool handle_gamepad()
{
        SDL_Event sdl_event;
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
                if (sdl_event.key.keysym.sym == SDLK_ESCAPE)
                    return true; // quit now
                if (sdl_event.key.keysym.sym == SDLK_PAUSE)
                    user_button=1;

                event_push((struct event){
                    .type= evt_keyboard_press,
                    .kbd={
                        .key=key_trans[sdl_event.key.keysym.scancode],
                        .mod=sdl_event.key.keysym.mod
                    }
                });

                break;

            case SDL_KEYUP:               
                if (sdl_event.key.keysym.sym == SDLK_PAUSE)
                    user_button=0;
                event_push((struct event) {
                    .type= evt_keyboard_release,
                    .kbd={
                        .key=key_trans[sdl_event.key.keysym.scancode],
                        .mod=sdl_event.key.keysym.mod
                    }
                });
                break;

            // joypads
            case SDL_JOYAXISMOTION: // analog position
                switch (sdl_event.jaxis.axis) {
                    case 0: /* X axis */
                        gamepad_x[0]=sdl_event.jaxis.value>>8;
                        break;
                    case 1: /* Y axis*/ 
                        gamepad_y[0]=sdl_event.jaxis.value>>8;
                        break;
                }
                break;

            case SDL_JOYBUTTONUP: // buttons
                if (sdl_event.jbutton.button>=gamepad_max_buttons || sdl_event.jbutton.which>=gamepad_max_pads)
                    break;
                gamepad_buttons[sdl_event.jbutton.which] &= ~(1<<sdl_event.jbutton.button);
                break;

            case SDL_JOYBUTTONDOWN:
                if (sdl_event.jbutton.button>=gamepad_max_buttons || sdl_event.jbutton.which>=gamepad_max_pads)
                    break;
                gamepad_buttons[sdl_event.jbutton.which] |= 1<<sdl_event.jbutton.button;

                break;

            case SDL_JOYHATMOTION: // HAT
                if (sdl_event.jbutton.which>=gamepad_max_pads)
                    break;
                gamepad_buttons[sdl_event.jbutton.which] &= ~(gamepad_up|gamepad_down|gamepad_left|gamepad_right);
                if (sdl_event.jhat.value & SDL_HAT_UP)      gamepad_buttons[sdl_event.jbutton.which] |= gamepad_up;
                if (sdl_event.jhat.value & SDL_HAT_DOWN)    gamepad_buttons[sdl_event.jbutton.which] |= gamepad_down;
                if (sdl_event.jhat.value & SDL_HAT_LEFT)    gamepad_buttons[sdl_event.jbutton.which] |= gamepad_left;
                if (sdl_event.jhat.value & SDL_HAT_RIGHT)   gamepad_buttons[sdl_event.jbutton.which] |= gamepad_right;
                break;

            // mouse 
            case SDL_MOUSEBUTTONDOWN:
                event_push((struct event){.type=evt_mouse_click, .button={.port=0, .id=sdl_event.button.button-1}});
                data_mouse_buttons |= 1<<(sdl_event.button.button-1);
                break;
            case SDL_MOUSEBUTTONUP:
                event_push((struct event){.type=evt_mouse_release, .button={.port=0, .id=sdl_event.button.button-1}});
                data_mouse_buttons &= ~1<<(sdl_event.button.button-1);
                break;
            case SDL_MOUSEMOTION :
                event_push( (struct event){
                    .type=evt_mouse_move, 
                    .mov={.port=0, .x=sdl_event.motion.xrel, .y=sdl_event.motion.yrel}
                });
                data_mouse_x += sdl_event.motion.xrel;
                data_mouse_y += sdl_event.motion.yrel;
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
    struct dirent *de = readdir((NIX_DIR *)dp->fs); // updates ?
    if (de) {
        for (int i=0;i<13;i++)
            fno->fname[i]=de->d_name[i];
        return FR_OK;
    } else {
        printf("Error reading directory %s : %s\n",dp->dir, strerror(errno));
        return FR_DISK_ERR;
    }
}


// ------------- datacopy
void *memcpy2(void *dst, void*src, size_t size) 
{
    return memcpy(dst,src,size);
}

// user button
int button_state() {
    return user_button;
}

// user LED
void set_led(int x) {
    printf("Setting LED to %d\n",x);
    // do nothing : set keyboard LED ? window title ?
}

int main ( int argc, char** argv )
{

    fullscreen = (argc>1 && !strcmp(argv[1],"--fullscreen"));
    if (fullscreen) 
        printf("Running fullscreen.\n");
    else 
        printf("Invoke with --fullscreen argument to run fullscreen.\n");

    slow = (argc>1 && !strcmp(argv[1],"--slow"));
    if (slow) 
        printf("Running slow.\n");
    else 
        printf("Invoke with --slow argument to run slower (fullscreen not supported).\n");

    quiet = (argc>1 && !strcmp(argv[1],"--quiet"));

    if (!quiet) {
        instructions();
        printf(" - Starting\n");        
    }

    gamepad_buttons[0] = 0; // all up
    gamepad_buttons[1] = 0;
    
    if (init()) return 1;
    game_init();

    // now start sound
    SDL_PauseAudio(0);

    // program main loop
    bool done = false;
    while (!done)
    {

        // message processing loop
        done = handle_gamepad();
        // update game
        game_frame();

        // update time
        vga_frame++;

        refresh_screen(screen);

        SDL_Delay(time_left());
        next_time += slow ? TICK_INTERVAL*10:TICK_INTERVAL;

        SDL_Flip(screen);
    } // end main loop

    // all is well ;)
    if (!quiet)
        printf(" - Exited cleanly\n");
    return 0;
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
    exit(0);
}
