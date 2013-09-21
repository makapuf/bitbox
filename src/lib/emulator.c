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
SDL_Surface* screen;
uint16_t mybuffer[MAX_SCREEN_WIDTH+MARGIN*2];
pixel_t *draw_buffer = mybuffer; // volatile ?
volatile uint16_t gamepad1;
uint32_t line; 
volatile uint32_t frame;

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
0000BBBBGGGGRRRR to 16bit color (565)
RRRR0GGGG00BBBB0
*/
inline uint16_t pixelconv(uint16_t pixel)
{
    return (pixel&0x00f)<<12 | (pixel & 0xf0)<<3 | (pixel & 0xf00)>>7;
}


static void refresh_screen(SDL_Surface *scr)
// uses global line
{
    uint16_t *dst = (uint16_t*)scr->pixels;

    for (line=0;line<screen_height;line++) {
        game_line(); // using line, updating draw_buffer ...

        // copy to screen at this position (cheating)
        uint16_t *src = (uint16_t*) &draw_buffer[0];//[MARGIN];

        for (int i=0;i<screen_width;i++) {
            *dst++= pixelconv(*src++);
        }
    }

}

static void mixaudio(void * userdata, Uint8 * stream, int len)
 {
     return;
 }

static int init_audio(void)
{
   SDL_AudioSpec desired, obtained;

   desired.freq = 31469;
   desired.format = AUDIO_U8;
   desired.channels = 1;

   /* Le tampon audio contiendra 512 échantillons */
   desired.samples = 512;

   /* Mise en place de la fonction de rappel et des données utilisateur */
   desired.callback = &mixaudio;
   desired.userdata = NULL;

   if (SDL_OpenAudio(&desired, &obtained) != 0) {
     printf("Erreur lors de l'ouverture du périphérique audio: %s\n", SDL_GetError());
     return 1;
   }

   printf("Paramètres audio obtenus: format %d,%d canaux, fs=%d, %d samples.\n",
 	 obtained.format & 0xff, obtained.channels, obtained.freq, obtained.samples);
    return 0;

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
    if (init_audio()) {
        printf("Unable to get audio!\n");
        return 2;
    }

    printf("screen is now %dx%d\n",screen_width,screen_height);
    return 0;
}

void instructions ()
{
    printf("Use following keys : ");
    printf("ESC (quit),");
    printf("Space (A fire),");
    printf("C (B fire),");
    printf("V (X fire),");
    printf("B (Y fire),");
    printf("Left/Right CTRL (L/R shoulders),");
    printf("Enter (start)");
    printf("Arrows (D-pad).\n");

}

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
                        KD(SPACE, A)
                        KD(RETURN, start)
                        KD(LCTRL, L)
                        KD(RCTRL, R)
                        KD(RIGHT, right)
                        KD(LEFT, left)
                        KD(UP, up)
                        KD(DOWN, down)
                        KD(c,B)
                        KD(v,X)
                        KD(b,Y)
                        default :
                        break;

                    }
                    break;
                }
            case SDL_KEYUP:
                {
                    switch(event.key.keysym.sym) {
                        KU(SPACE, A)
                        KU(RETURN, start)
                        KU(LCTRL, L)
                        KU(RCTRL, R)
                        KU(RIGHT, right)
                        KU(LEFT, left)
                        KU(UP, up)
                        KU(DOWN, down)
                        KU(c,B)
                        KU(v,X)
                        KU(b,Y)

                        default :
                        break;

                    }
                    break;

                }
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


    // program main loop
    bool done = false;
    while (!done)
    {

        // message processing loop
        done = handle_gamepad();
        // update game

        // update time
        frame++;
        
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

Sample *sample;
int sample_id;

void audio_start_sample(Sample *s)
{
    sample=s;
    sample_id=0;
}
void audio_play_sample() {}


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