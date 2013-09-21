#pragma once
#include <stdint.h>
#include "gamepad.h"
#include "audio.h"

// 0000RRRRGGGGBBBB

#define LINE_LENGTH 640 // attn this is not the size of the buffer since margins are added (easier to handle deltas)
#define MARGIN  32
#define MAX_SCREEN_WIDTH 1024-2*MARGIN // inpixels
#define TILEDATA_ALIGN 16

typedef uint16_t  pixel_t;

inline uint16_t RGB(uint8_t r, uint8_t g, uint8_t b) 
{
    return (((b)&0xf)<<8 | ((g)&0xf)<<4 | ((r)&0xf));
}

void game_init(void);
void game_frame(void);
void game_line(void);

void game_sample(void); // this callback is called each time we need a new audio sample  ! <-- XXX put in line ??
void die(char *msg);


// more pixels to allow over blitting XXX put too enemies_data : define as tile size max !
// tile data is aligned as (and indices represents N bytes) - XXX to shoot_data

extern pixel_t *draw_buffer;
extern uint32_t line;
extern volatile uint32_t frame;

void vga640_setup();

// perfs counters
extern uint32_t max_line,max_line_time; // max_line is the maximum number of cycles used on last frame.

