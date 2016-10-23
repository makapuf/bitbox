#include "bitbox.h"

int i;

int main() {
	while(1) {
		set_led(button_state() || (i++ & 1<<20));
	}
}