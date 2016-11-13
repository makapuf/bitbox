#include "bitbox.h"

__attribute__((weak)) int main(void)
{
	game_init();

	while (1) {

		// wait next frame.
		#ifndef NO_VGA
		int oframe=vga_frame;
		while (oframe==vga_frame);
		#endif

		game_frame();
		//set_led(button_state());
	}
}
