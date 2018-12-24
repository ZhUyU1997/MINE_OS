#include "lcd.h"
#include "lcd_controller.h"
#include "s3c24xx.h"

//TODO:fix redefined
#define HCLK 100

void jz2440_lcd_pin_init(void) {
	/* 初始化引脚 : 背光引脚 */
	GPBCON &= ~0x3;
	GPBCON |= 0x01;
	GPBDAT  &= ~(1 << 0);	// Power off

	/* LCD专用引脚 */
	//GPC5为USB使能引脚，需要过滤掉
	GPCUP   &= 0x1 << 5;
	GPCUP   |= ~(0x1 << 5); // 禁止内部上拉
	GPCCON	&= 0x3 << 10;
	GPCCON  |= 0xaaaaa2aa;	// GPIO管脚用于VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND
	//GPDUP   = 0xffffffff;	// 禁止内部上拉
	GPDCON  = 0xaaaaaaaa;	// GPIO管脚用于VD[23:8]

	/* PWREN */
	GPGCON |= (3 << 8);
}


/* 根据传入的LCD参数设置LCD控制器 */
void s3c2440_lcd_controller_init(struct lcd_params *lcdparams) {
	int pixelplace;
	unsigned int addr;

	jz2440_lcd_pin_init();

	/* [17:8]: clkval, vclk = HCLK / [(CLKVAL+1) x 2]
	 *                   9   = 100M /[(CLKVAL+1) x 2], clkval = 4.5 = 5
	 *                 CLKVAL = 100/vclk/2-1
	 * [6:5]: 0b11, tft lcd
	 * [4:1]: bpp mode
	 * [0]  : LCD video output and the logic enable/disable
	 */
	//TODO:计算不准确，可能会导致问题
	int clkval = HCLK / lcdparams->time_seq.vclk / 2 - 1;

	//int clkval = 5;
	int bppmode = lcdparams->bpp == 8  ? 0xb : \
				  lcdparams->bpp == 16 ? 0xc : \
				  0xd;  /* 0xd: 24,32bpp */
	LCDCON1 = (clkval << 8) | (3 << 5) | (bppmode << 1) ;

	/* [31:24] : VBPD    = tvb - 1
	 * [23:14] : LINEVAL = line - 1
	 * [13:6]  : VFPD    = tvf - 1
	 * [5:0]   : VSPW    = tvp - 1
	 */
	LCDCON2 =	((lcdparams->time_seq.tvb - 1) << 24) | \
				((lcdparams->yres - 1) << 14)         | \
				((lcdparams->time_seq.tvf - 1) << 6)  | \
				((lcdparams->time_seq.tvp - 1) << 0);

	/* [25:19] : HBPD	 = thb - 1
	 * [18:8]  : HOZVAL  = 列 - 1
	 * [7:0]   : HFPD	 = thf - 1
	 */
	LCDCON3 =	((lcdparams->time_seq.thb - 1) << 19) | \
				((lcdparams->xres - 1) << 8)		      | \
				((lcdparams->time_seq.thf - 1) << 0);

	/*
	 * [7:0]   : HSPW	 = thp - 1
	 */
	LCDCON4 =	((lcdparams->time_seq.thp - 1) << 0);

	/* 用来设置引脚极性, 设置16bpp, 设置内存中象素存放的格式
	 * [12] : BPP24BL
	 * [11] : FRM565, 1-565
	 * [10] : INVVCLK, 0 = The video data is fetched at VCLK falling edge
	 * [9]  : HSYNC是否反转
	 * [8]  : VSYNC是否反转
	 * [7]  : INVVD, rgb是否反转
	 * [6]  : INVVDEN
	 * [5]  : INVPWREN
	 * [4]  : INVLEND
	 * [3]  : PWREN, LCD_PWREN output signal enable/disable
	 * [2]  : ENLEND
	 * [1]  : BSWP
	 * [0]  : HWSWP
	 */

	pixelplace = lcdparams->bpp == 32 ? (0) : \
				 lcdparams->bpp == 16 ? (1) : \
				 (1 << 1); /* 8bpp */

	LCDCON5 = (lcdparams->pins_pol.vclk << 10) | \
			  (lcdparams->pins_pol.rgb << 7)   | \
			  (lcdparams->pins_pol.hsync << 9) | \
			  (lcdparams->pins_pol.vsync << 8) | \
			  (lcdparams->pins_pol.de << 6)    | \
			  (lcdparams->pins_pol.pwren << 5) | \
			  (1 << 11) | pixelplace;

	/* framebuffer地址 */
	/*
	 * [29:21] : LCDBANK, A[30:22] of fb
	 * [20:0]  : LCDBASEU, A[21:1] of fb
	 */
	addr = lcdparams->fb_base & ~(1 << 31);
	LCDSADDR1 = (addr >> 1);

	/*
	 * [20:0] : LCDBASEL, A[21:1] of end addr
	 */
	addr = lcdparams->fb_base + lcdparams->xres * lcdparams->yres * lcdparams->bpp / 8;
	addr >>= 1;
	addr &= 0x1fffff;
	LCDSADDR2 = addr;//
}

void s3c2440_lcd_controller_enalbe(void) {
	/* 背光引脚 : GPB0 */
	GPBDAT |= (1 << 0);

	/* pwren    : 给LCD提供AVDD  */
	LCDCON5 |= (1 << 3);

	/* LCDCON1'BIT 0 : 设置LCD控制器是否输出信号 */
	LCDCON1 |= (1 << 0);
}

void s3c2440_lcd_controller_disable(void) {
	/* 背光引脚 : GPB0 */
	GPBDAT &= ~(1 << 0);

	/* pwren	: 给LCD提供AVDD  */
	LCDCON5 &= ~(1 << 3);

	/* LCDCON1'BIT 0 : 设置LCD控制器是否输出信号 */
	LCDCON1 &= ~(1 << 0);
}


/* 设置调色板之前, 先关闭lcd_controller */
void s3c2440_lcd_controller_init_palette(void) {
	volatile unsigned int *palette_base = (volatile unsigned int *)0x4D000400;
	int i;

	int bit = LCDCON1 & (1 << 0);

	/* LCDCON1'BIT 0 : 设置LCD控制器是否输出信号 */
	if (bit)
		LCDCON1 &= ~(1 << 0);

	for (i = 0; i < 256; i++) {
		/* 低16位 : rgb565 */
		*palette_base++ = i;
	}

	if (bit)
		LCDCON1 |= (1 << 0);
}

struct lcd_controller s3c2440_lcd_controller = {
	.name    = "s3c2440",
	.init    = s3c2440_lcd_controller_init,
	.enable  = s3c2440_lcd_controller_enalbe,
	.disable = s3c2440_lcd_controller_disable,
	.init_palette = s3c2440_lcd_controller_init_palette,
};


void s3c2440_lcd_contoller_add(void) {
	register_lcd_controller(&s3c2440_lcd_controller);
}

