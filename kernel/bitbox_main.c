#include "bitbox.h"

__attribute__((weak)) void bitbox_main(void)
{
	game_init();

	while (1) {

		// wait next frame.
		#if VGA_MODE!=NONE
		wait_vsync(1);
		#endif

		game_frame();
		//set_led(button_state());
	}
}


// redefined in emu 
__attribute__((weak)) void wait_vsync(const unsigned int n)
{
    uint32_t nframe = vga_frame+n;
    while (vga_frame < nframe) {
    }
}
