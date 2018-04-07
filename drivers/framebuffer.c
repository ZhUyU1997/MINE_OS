/*
 * FILE: framebuffer.c
 * 实现在framebuffer上画点、画线、画同心圆、清屏的函数
 */
#include "framebuffer.h"

extern unsigned int fb_base_addr;
extern unsigned int bpp;
extern unsigned int xsize;
extern unsigned int ysize;
static unsigned int text_color = 0x0;
static unsigned int background_color = 0xffffff;
/*
 * 画点
 * 输入参数：
 *     x、y : 象素坐标
 *     color: 颜色值
 */
void PutPixel(UINT32 x, UINT32 y, UINT16 color) {
	//TODO:
	if (x < 0 || x > xsize - 1 || y < 0 || y > ysize - 1) {
		printf("PutPixel error!!\n");
		return;
	}
	UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
	*addr = (UINT16) color;
}
UINT16 GetPixel(UINT32 x, UINT32 y) {
	//TODO:
	if (x < 0 || x > xsize - 1 || y < 0 || y > ysize - 1) {
		printf("GetPixel error!!\n");
		return 0;
	}
	UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
	return *addr;
}

/*
 * 将屏幕清成单色
 * 输入参数：
 *     color: 颜色值
 */
void ClearScr(UINT16 color) {
	UINT32 x, y;

	for (y = 0; y < ysize; y++)
		for (x = 0; x < xsize; x++)
			PutPixel(x, y, color);
}
#define FONTDATAMAX 2048

void lcd_set_text_color(unsigned int color) {
	text_color = color;
}
void lcd_set_background_color(unsigned int color) {
	background_color = color;
}
