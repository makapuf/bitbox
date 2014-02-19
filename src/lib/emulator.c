#include <stdlib.h>
#include <SDL/SDL.h>
#include <stdbool.h>

#include "kernel.h"

//#include "object.h"

// gerer SLOW et FULLSCREEN comme commandes de l'emulateur
//#define SLOW
// ----------------------------- kernel ----------------------------------
/* The only funcion of the kernel is
- calling game_init and game_frame
- calling update draw_buffer (which calls object_blitX)

- displaying other buffer updating line, frame
- read user input and update gamepad1

*/

// ticks in ms

int screen_width;
int screen_height;

#ifndef SLOW
#define TICK_INTERVAL 1000/60
#else
#define TICK_INTERVAL 1000/6
#endif


static uint32_t next_time;

// Video
SDL_Surface* screen;
uint16_t mybuffer[LINE_LENGTH];
pixel_t *draw_buffer = mybuffer; // volatile ?
volatile uint16_t gamepad1;
uint32_t vga_line; 
volatile uint32_t vga_frame;

// sound
uint16_t audio_buffer[BITBOX_SNDBUF_LEN]; // stereo, 31khz 1frame
int audio_on;

// joystick handling.
static int sdl_joy_num;
static SDL_Joystick * sdl_joy = NULL;
static const int joy_commit_range = 3276;


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

    for (vga_line=0;vga_line<screen_height;vga_line++) {
        game_line(); // using line, updating draw_buffer ...

        // copy to screen at this position (cheating)
        uint16_t *src = (uint16_t*) &draw_buffer[0];//[MARGIN];

        for (int i=0;i<screen_width;i++) {
            *dst++= pixelconv(*src++);
        }
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
     printf("Erreur lors de l'ouverture du périphérique audio: %s\n", SDL_GetError());
     return ; // return anyways even with no sound
   }
   printf("Paramètres audio desired (after): format %d,%d canaux, fs=%d, %d samples.\n",
     desired.format , desired.channels, desired.freq, desired.samples);

}


void set_mode(int width, int height)
{
    screen_width = width;
    screen_height = height;
    screen = SDL_SetVideoMode(width,height, 16, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        printf("%s\n",SDL_GetError());
        die("Unable to set video mode:!\n");
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
    set_mode(640,480);
    audio_init();
    joy_init();


    printf("screen is now %dx%d\n",screen_width,screen_height);
    return 0;
}


void instructions ()
{
    printf("Invoke with FULLSCREEN argument to run fullscreen.\n");
    printf("Use joystick or following keys : \n");
    printf("    ESC (quit), \n");
    printf("    D (A button),\n");
    printf("    F (B button),\n");
    printf("    E (X button),\n");
    printf("    R (Y button),\n");
    printf("    Left/Right CTRL (L/R shoulders),\n");
    printf("    Space (Select),\n");
    printf("    Enter (start) \n");
    printf("    Arrows (D-pad)\n");
}



#define KD(key,code) case SDLK_##key : gamepad1 |= (1<<gamepad_##code); break;
#define KU(key,code) case SDLK_##key : gamepad1 &= ~(1<<gamepad_##code); break;



#define BUTMAX 8
const int8_t JOY_BUTTONS[BUTMAX] = 
{
    gamepad_B,
    gamepad_Y,
    gamepad_A,
    gamepad_X,
    gamepad_L,
    gamepad_start,
    gamepad_R,
    gamepad_select,
};

static void handle_joystick(SDL_Event event)
{
    int axisval = event.jaxis.value;
    switch (event.jaxis.axis)
    {
    case 0: /* X axis */
        if (axisval > joy_commit_range)
            gamepad1 |= (1<<gamepad_right);
        else 
            gamepad1 &= ~(1<<gamepad_right);

        if (axisval < -(joy_commit_range))
            gamepad1 |= (1<<gamepad_left);
        else 
            gamepad1 &= ~(1<<gamepad_left);
        break;
        
    case 1: /* Y axis*/ 
        if (axisval > joy_commit_range)
            gamepad1 |= (1<<gamepad_down);
        else 
            gamepad1 &= ~(1<<gamepad_down);

        if (axisval < -(joy_commit_range))
            gamepad1 |= (1<<gamepad_up);
        else 
            gamepad1 &= ~(1<<gamepad_up);
        break;
    }
}

const char * gamepad_names[] = {
    [11]="gamepad_B",
    [10]="gamepad_Y",
    [9]="gamepad_select",
    [8]="gamepad_start",
    [7]="gamepad_up",
    [6]="gamepad_down",
    [5]="gamepad_left",
    [4]="gamepad_right",
    [3]="gamepad_A",
    [2]="gamepad_X",
    [1]="gamepad_L",
    [0]="gamepad_R",
};

#define KD(key,code) case SDLK_##key : gamepad1 |= (1<<gamepad_##code); break;
#define KU(key,code) case SDLK_##key : gamepad1 &= ~(1<<gamepad_##code); break;

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
                            return true; // quit
                        break;
                        KD(d,A)
                        KD(f,B)
                        KD(e,X)
                        KD(r,Y)

                        KD(LCTRL, L)
                        KD(RCTRL, R)

                        KD(RETURN, start)
                        KD(SPACE, select)

                        KD(RIGHT, right)
                        KD(LEFT, left)
                        KD(UP, up)
                        KD(DOWN, down)

                        default :
                        break;
                    }
                    break;
                }
            case SDL_KEYUP:
                {
                    switch(event.key.keysym.sym) {
                        KU(d, A)
                        KU(RETURN, start)
                        KU(LCTRL, L)
                        KU(RCTRL, R)
                        KU(RIGHT, right)
                        KU(LEFT, left)
                        KU(UP, up)
                        KU(DOWN, down)
                        KU(f,B)
                        KU(e,X)
                        KU(r,Y)
                        KU(SPACE, select)
                        default :
                        break;
                    }
                    break;

                }
            case SDL_JOYAXISMOTION:
                handle_joystick(event);
                break;
            case SDL_JOYBUTTONUP:
                if (event.jbutton.button>=BUTMAX) break;
                gamepad1 &= ~(1<<JOY_BUTTONS[event.jbutton.button]);
                break;
            case SDL_JOYBUTTONDOWN:
                if (event.jbutton.button>=BUTMAX) break;
                gamepad1 |= 1<<JOY_BUTTONS[event.jbutton.button];
                printf ("pressed button %d : %s\n",
                    event.jbutton.button,
                    gamepad_names[JOY_BUTTONS[event.jbutton.button]]
                    );
                break;
            } // end switch
        } // end of message processing

        return false; // don't exit  now
}

int main ( int argc, char** argv )
{
    instructions();

    gamepad1 = 0; // all up
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

        // update time
        vga_frame++;
        
        game_frame();

        refresh_screen(screen);

        SDL_Delay(time_left());
        next_time += TICK_INTERVAL;

        SDL_Flip(screen);
    } // end main loop

    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}


void die(char *str)
{
    printf("ERROR : %s - dying.",str);
    exit(0);
}

/*
void displaylist_print(void)
// debug function
{
    printf("alive objects :");

    for (Object *o=displaylist_top;o;o=o->displaylist_next)
        printf("  %p (%s) z=%3d,x=%3d,y=%3d\n",o,o->type->name,o->type->zlevel,o->x,o->y);

    printf("\n");
}
*/