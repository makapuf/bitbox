#pragma once

#include <stdint.h>

typedef enum {
    gamepad_B=11,
    gamepad_Y=10,
    gamepad_select=9,
    gamepad_start=8,
    gamepad_up=7,
    gamepad_down=6,
    gamepad_left=5,
    gamepad_right=4,
    gamepad_A=3,
    gamepad_X=2,
    gamepad_L=1,
    gamepad_R=0,
    gamepad_MAX=12,
} Gamepad;
// inverted cause we read data by shifting left

#define PRESSED(key) (gamepad1 & (1<<gamepad_##key))

extern volatile uint16_t gamepad1;
void gamepad_init();

/*
 *  does a step to read whole value. Steps need be clocked at least 200ns.
 *  after 34 steps, value is read into gamepad_state.
 */
void gamepad_readstep();

