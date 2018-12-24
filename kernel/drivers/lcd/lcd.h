#ifndef _LCD_H
#define _LCD_H

/*
 * NORMAL : 正常极性
 * INVERT : 反转极性
 */
enum POLARITY {
	NORMAL = 0,
	INVERT = 1,
};



struct pins_polarity {
	int de;    /* normal: 高电平时可以传输数据 */
	int pwren; /* normal: 高电平有效 */
	int vclk;  /* normal: 在下降沿获取数据 */
	int rgb;   /* normal: 高电平表示1 */
	int hsync; /* normal: 高脉冲 */
	int vsync; /* normal: 高脉冲 */
};

struct time_sequence {
	/* 垂直方向 */
	int tvp; /* vysnc脉冲宽度 */
	int tvb; /* 上边黑框, Vertical Back porch */
	int tvf; /* 下边黑框, Vertical Front porch */

	/* 水平方向 */
	int thp; /* hsync脉冲宽度 */
	int thb; /* 左边黑框, Horizontal Back porch */
	int thf; /* 右边黑框, Horizontal Front porch */

	int vclk;
};


struct lcd_params {
	char *name;

	/* 引脚极性 */
	struct pins_polarity pins_pol;

	/* 时序 */
	struct time_sequence time_seq;

	/* 分辨率, bpp */
	int xres;
	int yres;
	int bpp;

	/* framebuffer的地址 */
	unsigned int fb_base;
};

void get_lcd_params(unsigned int *fb_base, int *xres, int *yres, int *bpp);

#endif /* _LCD_H */


