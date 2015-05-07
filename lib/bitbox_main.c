#include "bitbox.h"

__attribute__((weak)) int main(void) 
{
	game_init();

	while (1) {
		game_frame();
		
		// wait next frame.
		int oframe=vga_frame;
		while (oframe==vga_frame);
			
		set_led(button_state());
	} 
}
