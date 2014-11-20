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

SDL_Surface* screen;
uint16_t mybuffer1[LINE_BUFFER];
uint16_t mybuffer2[LINE_BUFFER];
uint16_t *draw_buffer; // volatile ?
volatile uint16_t gamepad_buttons[2]; 
uint32_t vga_line; 
volatile uint32_t vga_frame;

volatile int data_mouse_x, data_mouse_y;
volatile uint8_t data_mouse_buttons;

int user_button=0; 

// sound
uint16_t audio_buffer[BITBOX_SNDBUF_LEN]; // stereo, 31khz 1frame
int audio_on;

// joystick handling.
static int sdl_joy_num;
static SDL_Joystick * sdl_joy = NULL;
static const int joy_commit_range = 3276;
static const int gamepad_MAX = 12;

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
// uses global line
{
    uint16_t *dst = (uint16_t*)scr->pixels;
    
    draw_buffer = &mybuffer1[0];
    graph_frame();

    for (vga_line=0;vga_line<screen_height;vga_line++) {
        graph_line(); // using line, updating draw_buffer ...

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
    if (audio_on)
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
   
   printf("sndbuflen : %d\n",BITBOX_SNDBUF_LEN);
   printf("Paramètres audio desired (before): format %d,%d canaux, fs=%d, %d samples.\n",
     desired.format , desired.channels, desired.freq, desired.samples);

    //if (SDL_OpenAudio(&desired, &obtained) != 0) {
   if (SDL_OpenAudio(&desired, NULL) != 0) {
     printf("Error in opening audi peripheral: %s\n", SDL_GetError());
     return ; // return anyways even with no sound
   }
   printf("Audio parameters desired (after): format %d,%d canaux, fs=%d, %d samples.\n",
     desired.format , desired.channels, desired.freq, desired.samples);

}


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

    /* now try and open one. If, for some reason it fails, move on to the next one */
    for (i = 0; i < joy_count; i++)
    {
        sdl_joy = SDL_JoystickOpen(i);
        if (sdl_joy)
        {
            sdl_joy_num = i;
            break;
        }   
    }
    
    /* make sure that Joystick event polling is a go */
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
    printf("Use joystick or following keys : \n");
    printf("       -------\n");
    printf("    Space (Select),   Enter (Start),   Arrows (D-pad)\n");
    printf("    D (A button),     F (B button),    E (X button),   R (Y button),\n");
    printf("    Left/Right CTRL (L/R shoulders),   ESC (quit), B (user Button)\n");
    printf("       -------\n");
}



// handle HAT and analog joystick properly.


static void handle_joystick(SDL_Event event)
{
    int axisval = event.jaxis.value;
    switch (event.jaxis.axis)
    {
    case 0: /* X axis */
        if (axisval > joy_commit_range)
            gamepad_buttons[0] |= gamepad_right;
        else 
            gamepad_buttons[0] &= ~gamepad_right;

        if (axisval < -(joy_commit_range))
            gamepad_buttons[0] |= gamepad_left;
        else 
            gamepad_buttons[0] &= ~gamepad_left;
        break;
        
    case 1: /* Y axis*/ 
        if (axisval > joy_commit_range)
            gamepad_buttons[0] |= gamepad_down;
        else 
            gamepad_buttons[0] &= ~gamepad_down;

        if (axisval < -(joy_commit_range))
            gamepad_buttons[0] |= gamepad_up;
        else 
            gamepad_buttons[0] &= ~gamepad_up;
        break;
    }
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

#define KD(key,code) case SDLK_##key : gamepad_buttons[0] |= gamepad_##code; break;
#define KU(key,code) case SDLK_##key : gamepad_buttons[0] &= ~gamepad_##code; break;

static bool handle_gamepad()
{
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
            case SDL_QUIT:
                return true;
                break;

                // check for keypresses
            case SDL_KEYDOWN:
                {
                    switch(event.key.keysym.sym) {
                        // exit if ESCAPE is pressed
                        case SDLK_ESCAPE:
                            return true; // quit now
                        break;
                        KD(d,A)
                        KD(f,B)
                        KD(e,X)
                        KD(r,Y)
                        KD(LCTRL, L)
                        KD(RCTRL, R)
                        KD(RETURN, start)
                        KD(SPACE, select)
                        KD(LEFT, left)
                        KD(RIGHT, right)
                        KD(UP, up)
                        KD(DOWN, down)

                        case SDLK_b : user_button = 1;
                        break;

                        default :
                        break;
                    }
                    break;
                }
            case SDL_KEYUP:
                {
                    switch(event.key.keysym.sym) {
                        KU(d, A)
                        KU(f,B)
                        KU(e,X)
                        KU(r,Y)
                        KU(LCTRL, L)
                        KU(RCTRL, R)
                        KU(RETURN, start)
                        KU(SPACE, select)
                        KU(LEFT, left)
                        KU(RIGHT, right)
                        KU(UP, up)
                        KU(DOWN, down)

                        case SDLK_b : user_button = 0;
                        break;

                        default :
                        break;
                    }
                    break;

                }
            case SDL_JOYAXISMOTION:
                handle_joystick(event);
                break;
            case SDL_JOYBUTTONUP:
                if (event.jbutton.button>=gamepad_MAX) break;
                gamepad_buttons[0] &= ~(1<<event.jbutton.button);
                break;
            case SDL_JOYBUTTONDOWN:
                if (event.jbutton.button>=gamepad_MAX) break;
                gamepad_buttons[0] |= 1<<event.jbutton.button;
                printf ("pressed button %d : %s\n",
                    event.jbutton.button,
                    gamepad_names[event.jbutton.button]
                    );
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
    fp->fs = (FATFS*) fopen ((const char*)path,"r"); // now ignores mode.
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
    *br = fread ( buff, 1, btr, (FILE *)fp->fs);
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

    instructions();
    printf(" - Starting\n");

    gamepad_buttons[0] = 0; // all up
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
