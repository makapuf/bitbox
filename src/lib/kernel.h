#pragma once
#include <stdint.h>
#include "gamepad.h"
#include "audio.h"

// 0RRRRRGGGGGBBBBB
// todo : sound, sd, user_led, user_button

#define LINE_LENGTH 640 

#define TILEDATA_ALIGN 16

typedef uint16_t  pixel_t;

inline uint16_t RGB(uint8_t r, uint8_t g, uint8_t b) 
{
    return (((b)&0xf)<<8 | ((g)&0xf)<<4 | ((r)&0xf));
}

void game_init(void);
void game_frame(void);
void game_line(void);

void game_snd_buffer(uint16_t *buffer, int len); // this callback is called each time we need to fill the buffer


void die(char *msg);


// more pixels to allow over blitting XXX put too enemies_data : define as tile size max !
// tile data is aligned as (and indices represents N bytes) - XXX to shoot_data

extern pixel_t *draw_buffer;
extern uint32_t vga_line;
extern volatile uint32_t vga_frame;

void vga640_setup();

// perfs counters
extern uint32_t max_line,max_line_time; // max_line is the maximum number of cycles used on last frame.

