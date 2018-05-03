#include "math.h"
#include "common.h"
#define IS_FIFO_READY (0x1<<7)

void send_iis_data(short data) {
	while (IISCON & IS_FIFO_READY);
	IISFIFO = data;
}


#define PI 3.1415926
static int fs = 44100;
static void sound(int second, int f) {
	double x = PI * f / fs;
	int time[9] = {0, 1, 2, 0, 4, 0, 0, 0, 8};
	for (int i = 0; i < fs * time[second] / 8; i++) {
		double ii = i * 5 / (fs * 1 / 8);
		int h = 12000 * ii * ii / exp(ii) + 0.5 * (1 - 1 / (ii + 1));
		double temp = 1000 * sin(x * i) + 32768;
		unsigned short t = temp;
		send_iis_data(t);
		send_iis_data(t);
	}
}
static int play(short *buf, unsigned int len) {
	short *pData = buf;
	short *pData_end = buf + len;
	for (unsigned int i = 0; i < len ; i++)
		send_iis_data(pData[i]);
	return 0;

}

static void mtest() {
	int f[8] = {0, 494, 554, 622, 659, 740, 881, 932};
	int fh[8] = {0, 988, 1109, 1245, 1318, 1480, 1661, 1865};
	int fhh[8] = {0, 1975, 1109, 1245, 1318, 1480, 1661, 1865};
	int p[4] = {1, 2, 4, 8};
	sound(p[1], f[5]);
	sound(p[1], fh[1]);
	sound(p[1], fh[2]);

	sound(p[1], fh[2]);
	sound(p[1], fh[3]);
	sound(p[1], fh[3]);
	sound(p[1], f[0]);
	sound(p[1], fh[2]);
	sound(p[1], fh[3]);
	sound(p[1], fh[3]);
	sound(p[1], f[0]);

	sound(p[1], fh[2]);
	sound(p[1], fh[3]);
	sound(p[1], fh[5]);
	sound(p[1], fh[3]);
	sound(p[1], fh[1]);
	sound(p[1], f[0]);
	sound(p[1], fh[1]);
	sound(p[1], f[5]);

	sound(p[2], f[6]);
	sound(p[1], f[0]);
	sound(p[1], fh[3]);
	sound(p[2], fh[2]);
	sound(p[1], fh[1]);
	sound(p[1], fh[2]);
//
	sound(p[3], fh[3]);
	sound(p[1], f[0]);
	sound(p[1], f[5]);
	sound(p[1], fh[1]);
	sound(p[1], fh[2]);

	sound(p[1], fh[2]);
	sound(p[1], fh[3]);
	sound(p[1], fh[3]);
	sound(p[1], f[0]);
	sound(p[1], fh[2]);
	sound(p[1], fh[3]);
	sound(p[1], fh[3]);
	sound(p[1], f[0]);

	sound(p[1], fh[2]);
	sound(p[1], fh[3]);
	sound(p[1], fh[5]);
	sound(p[1], fh[3]);
	sound(p[1], fh[1]);
	sound(p[1], f[0]);
	sound(p[1], fh[1]);
	sound(p[1], f[7]);

	sound(p[2], f[6]);
	sound(p[1], f[0]);
	sound(p[1], fh[3]);
	sound(p[2], fh[2]);
	sound(p[1], fh[1]);
	sound(p[1], fh[2]);

	sound(p[3], fh[1]);
	sound(p[2], f[0]);
	sound(p[1], fh[1]);
	sound(p[1], f[7]);


	sound(p[2], f[6]);
	sound(p[2], fh[1]);
	sound(p[1], fh[2]);
	sound(p[1], f[0]);
	sound(p[1], fh[1]);
	sound(p[1], fh[2]);

//
	sound(p[1], fh[3]);
	sound(p[1], fh[5]);
	sound(p[1], fh[2]);
	sound(p[1], fh[3]);
	sound(p[1], fh[1]);
	sound(p[1], f[0]);
	sound(p[1], fh[1]);
	sound(p[1], f[7]);

	sound(p[2], f[6]);
	sound(p[2], fh[1]);
	sound(p[1], fh[2]);
	sound(p[0], fh[3]);
	sound(p[0], fh[2]);
	sound(p[1], fh[1]);
	sound(p[1], fh[2]);

	sound(p[3], fh[3]);
	sound(p[2], f[0]);
	sound(p[1], fh[1]);
	sound(p[1], f[7]);

//
	sound(p[2], f[6]);
	sound(p[1], fh[1]);
	sound(p[1], f[0]);
	sound(p[1], fh[2]);
	sound(p[0], fh[3]);
	sound(p[0], fh[2]);
	sound(p[1], fh[1]);
	sound(p[1], fh[2]);

	sound(p[1], fh[3]);
	sound(p[1], fh[5]);
	sound(p[1], fh[5]);
	sound(p[1], fh[3]);
	sound(p[1], fh[6]);
	sound(p[1], f[0]);
	sound(p[1], fh[3]);
	sound(p[1], fh[2]);

	sound(p[2], fh[1]);
	sound(p[1], f[0]);
	sound(p[1], fh[3]);
	sound(p[1], fh[2]);
	sound(p[1], fh[2]);
	sound(p[1], fh[1]);
	sound(p[1], fh[2]);


//
	sound(p[3], fh[1]);
	sound(p[1], f[0]);
	sound(p[1], fh[1]);
	sound(p[1], fh[3]);
	sound(p[1], fh[4]);

	sound(p[1], fh[5]);
	sound(p[1], fh[1]);
	sound(p[1], fh[3]);
	sound(p[1], fh[4]);
	sound(p[2], fh[5]);
	sound(p[1], fh[6]);
	sound(p[1], fh[7]);

	sound(p[1], fhh[1]);
	sound(p[1], fh[3]);
	sound(p[1], fh[3]);
	sound(p[1], fh[4]);
	sound(p[2], fh[5]);
	sound(p[2], f[0]);
}
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
void mplay() {
	open_sound();
	while (1)mtest();
	close_sound();
}
