#include <stdlib.h>
/*
This module aims at testing games. 

It builds a standalone emulator, which run headless 
 and does not require SDL or produce any output, 
 but will stress the game for a moment and exercise , on a pc
 - game_frame, game_line, game_init
 - graph_frame, graph_line
 - game_snd_buffer

and provide bitbox API stubs 

*/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

// emulated interfaces
#include "bitbox.h"

#define DIR NIX_DIR // prevents name clashes with datfs DIR
#include <dirent.h>
#undef DIR

#include "fatfs/ff.h"


// ----------------------------- kernel ----------------------------------
/* The only function of the kernel is
- calling game_init and game_frame
- calling update draw_buffer (which calls object_blitX)

- displaying other buffer updating line, frame
- read user input and update gamepad ( mashing random buttons )

*/

// ticks in ms

int screen_width;
int screen_height;

#define TICK_INTERVAL 1000/60
#define LINE_BUFFER 1024

#define EMU_FRAMES 10*60*60 // 1 minute 
 
uint16_t audio[BITBOX_SNDBUF_LEN];
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


static void refresh_screen()
// uses global line + vga_odd
{

    draw_buffer = mybuffer1;
    graph_frame();

    for (vga_line=0;vga_line<screen_height;vga_line++) {
        vga_odd=0;
        graph_line(); // using line, updating draw_buffer ...
        #ifdef VGA_SKIPLINE
        vga_odd=1; graph_line(); //  a second time for SKIPLINE modes
        #endif 

        // swap lines buffers to simulate double line buffering
        draw_buffer = (draw_buffer == &mybuffer1[0] ) ? &mybuffer2[0] : &mybuffer1[0];
    }
}


// default empty implementation
__attribute__((weak)) void game_snd_buffer(uint16_t *buffer, int len)  {}

static void handle_gamepad()
// generate random gamepad events ?
{
    gamepad_x[0] = rand()&0xff;
    gamepad_y[0] = rand()&0xff;
    gamepad_buttons[0] = rand()&0xffff;
    // XXX generate random keyboard events ?
}

// -------------------------------------------------
// limited fatfs-related functions.
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

// user button
int button_state() {
    return user_button;
}

// user LED
void set_led(int x) {
    printf("Setting LED to %d\n",x);
    // do nothing : set keyboard LED ? window title ?
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

int main ( int argc, char** argv )
{
    printf("Starting test ... \n");

    gamepad_buttons[0] = 0; // all up
    gamepad_buttons[1] = 0;
    
    game_init();

    printf("  Game init done. \n");

    // program main loop
    for (int i=0;i<EMU_FRAMES;i++) {
        handle_gamepad();

        // update game
        game_frame();

        // update time
        vga_frame++;

        // one sound buffer per frame 
        game_snd_buffer(audio,BITBOX_SNDBUF_LEN); 

        refresh_screen();
    } // end main loop

    // all is well ;)
    printf("  Test OK !\n");
    return 0;
}
