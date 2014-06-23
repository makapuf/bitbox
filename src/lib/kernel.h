#pragma once
#include <stdint.h>
#include "gamepad.h"
#include "audio.h"

// 0RRRRRGGGGGBBBBB
// todo : sound, sd, user_led, user_button

#define LINE_LENGTH 640 

#define TILEDATA_ALIGN 16

typedef uint16_t  pixel_t;

#define RGB(r,g,b)  (((r>>3)&0x1f)<<10 | ((g>>3)&0x1f)<<5 | ((b>>3)&0x1f))


void game_init(void);
void game_frame(void);
void game_line(void);

void game_snd_buffer(uint16_t *buffer, int len); // this callback is called each time we need to fill the buffer

void die(int where, int cause);


// more pixels to allow over blitting XXX put too enemies_data : define as tile size max !
// tile data is aligned as (and indices represents N bytes) - XXX to shoot_data

extern pixel_t *draw_buffer;
extern uint32_t vga_line;
extern volatile uint32_t vga_frame;

void vga640_setup();

// perfs counters
extern uint32_t max_line,max_line_time; // max_line is the maximum number of cycles used on last frame.

void message(char *m);

void set_led(int value);
int button_state();
