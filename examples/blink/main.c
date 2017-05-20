#include "bitbox.h"

void graph_line() {}

int bitbox_main() {
	static int i;
	while(1) {
		set_led(button_state() || (i++ & 1<<20));
	}
}