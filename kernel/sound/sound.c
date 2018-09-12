#include "common.h"

void open_sound() {
	IISCON |= IISCON_INTERFACE_ENABLE;
}
void close_sound() {
	IISCON &= ~IISCON_INTERFACE_ENABLE;
}

void sound_init() {
	init_IIS_port();
	init_gpio_L3_port();
	init_wm8976();
	init_IIS_bus(44100);
	wm8976_set_volume(20);
}
