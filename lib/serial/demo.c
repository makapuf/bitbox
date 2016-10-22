// serial demo

#include "bitbox.h"
#include "serial.h"

int led;

void game_init() 
{
	serial_init();
    message ("Hello %s\n","all");
}

void game_frame()
{
    // if button pressed, send 'a' quickly
    if (button_state())
    	serial_putchar('a'); // low level output

    if (vga_frame%16) return;
    led=1-led;
    set_led(led);

    // echo input first character
    if (serial_rx_ready())
    	serial_putchar(serial_getchar()); // could block 
}

void graph_line() {}