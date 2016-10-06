// demo serial

#include "bitbox.h"
#include "serial.h"

int led;

void game_init() 
{
	serial_init();
}

void game_frame()
{
	if (vga_frame%16) return;
	led=1-led;
    set_led(led);

    // if button pressed, send 'a'
    if (button_state())
    	serial_putchar('a');

    // echo input first character
    if (serial_rx_ready())
    	serial_putchar(serial_getchar()); // could block 
}

void graph_line() {}