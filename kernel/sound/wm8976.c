#include <timer.h>
#include "common.h"
#include "s3c24xx.h"
#define L3M (1<<2)
#define L3D (1<<2)
#define L3C (1<<2)


static void wm8976_write_reg(unsigned char reg, unsigned int data) {
	//时序控制，wm8976手册p16
	int i;
	unsigned short val = (reg << 9) | (data & 0x1ff);

	GPBDAT |= GPB_X_WRITE(2);
	GPBDAT |= GPB_X_WRITE(3);
	GPBDAT |= GPB_X_WRITE(4);

	for (i = 0; i < 16; i++) {
		if (val & (1 << 15)) {
			GPBDAT &= GPX_X_CLEAR(4);
			GPBDAT |= GPB_X_WRITE(3);
			udelay(10);
			GPBDAT |= GPB_X_WRITE(4);
		} else {
			GPBDAT &= GPX_X_CLEAR(4);
			GPBDAT &= GPX_X_CLEAR(3);
			udelay(10);
			GPBDAT |= GPB_X_WRITE(4);
		}
		val = val << 1;
	}

	GPBDAT &= GPX_X_CLEAR(2);
	udelay(10);
	GPBDAT |= GPB_X_WRITE(2);
	GPBDAT |= GPB_X_WRITE(3);
	GPBDAT |= GPB_X_WRITE(4);
}

static volatile int sound_volume  = 0;
/*
 * volume : 0~100, 0表示最小音量
 */
void wm8976_set_volume(int volume) {
	//WM8976: 52,53号寄存器bit[5:0]表示音量, 值越大音量越大, 0-63
	if (volume > 100)
		volume = 100;
	if (volume < 0)
		volume = 0;
	int val = volume * 63 / 100;
	sound_volume = volume;
	wm8976_write_reg(52, (1 << 8) | val);
	wm8976_write_reg(53, (1 << 8) | val);
}
int wm8976_get_volume() {
	return sound_volume;
}

void wm8976_up_volume() {
	wm8976_set_volume(wm8976_get_volume() + 1);
}

void wm8976_down_volume() {
	wm8976_set_volume(wm8976_get_volume() - 1);
}

void init_wm8976(void) {
	//----------------------------------------
	//         Port B Group
	//Ports:	GPB2	GPB3	GPB4
	//Signal:	L3MODE	L3DATA	L3CLOCK
	//Setting:	OUTPUT	OUTPUT	OUTPUT
	//		[9:8]	[7:6]	[5:4]
	//Binary:	01	01	01
	//----------------------------------------
	//GPBUP = GPBUP | (0x7<<2);//disable GPB[4:2] pull up function
	//GPBCON = GPBCON & ~(0x3f<<4)|(0x15<<4);
	//GPBDAT = GPBDAT & ~(L3M|L3D|L3C) | (L3M|L3C);//L3Mode=H, L3Clock=H
	//WriteL3Data((0x3<<1)+1,0xef);//RMIXEN,LMIXEN,DACENR,DACENL
	//WriteL3Data((0x2<<1)+1,0x80);//ROUT1EN LOUT1EN
	/* software reset */
	wm8976_write_reg(0, 0);

	/* OUT2的左/右声道打开
	 * 左/右通道输出混音打开
	 * 左/右DAC打开
	 */
	wm8976_write_reg(0x3, 0x6f);
	wm8976_write_reg(0x1, 0x1f);//biasen,BUFIOEN.VMIDSEL=11b
	wm8976_write_reg(0x2, 0x185);//ROUT1EN LOUT1EN, inpu PGA enable ,ADC enable
	wm8976_write_reg(0x6, 0x0);//SYSCLK=MCLK
	wm8976_write_reg(0x4, 0x10);//16bit
	wm8976_write_reg(0x2B, 0x10); //BTL OUTPUT
	wm8976_write_reg(0x9, 0x50);//Jack detect enable
	wm8976_write_reg(0xD, 0x21);//Jack detect
	wm8976_write_reg(0x7, 0x01);//Jack detect
	wm8976_set_volume(0);
}
