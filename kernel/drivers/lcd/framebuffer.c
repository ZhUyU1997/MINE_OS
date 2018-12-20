#include <assert.h>
#include <graphics.h>
#include <memory.h>
#include "framebuffer.h"
#include "lcd.h"

/* 获得LCD参数 */
static unsigned int fb_base_addr;
static int xres, yres, bpp;

void fb_get_lcd_params(void)
{
	get_lcd_params(&fb_base_addr, &xres, &yres, &bpp);
}

/* 画点 */
void PutPixel(UINT32 x, UINT32 y, UINT16 color) {
	//TODO:
	if (x < 0 || x >= xres || y < 0 || y >= yres) {
		return;
	}
	UINT16 *addr = (UINT16 *)fb_base_addr + (y * xres + x);
	*addr = (UINT16) color;
}

UINT16 GetPixel(UINT32 x, UINT32 y) {
	//TODO:
	if (x < 0 || x >= xres || y < 0 || y >= yres) {
		return 0;
	}
	UINT16 *addr = (UINT16 *)fb_base_addr + (y * xres + x);
	return *addr;
}

/* 清屏 */
void ClearScreen(U32 color) {
	UINT32 x, y;

	color = convert888_565(color);
	for (y = 0; y < yres; y++)
		for (x = 0; x < xres; x++)
			PutPixel(x, y, color);
}

void MovLine(UINT32 dis_y, UINT32 src_y, UINT32 num) {
	UINT16 *src_addr = (UINT16 *)fb_base_addr + (src_y * xres);
	UINT16 *dis_addr = (UINT16 *)fb_base_addr + (dis_y * xres);
	memmove(dis_addr, src_addr, xres * num);
}

void ScrollLine(UINT32 top, UINT32 bottom, UINT32 num) {
	if(!num) return;
	UINT16 *top_addr = (UINT16 *)fb_base_addr + (top * xres);
	UINT16 *new_addr = (UINT16 *)fb_base_addr + ((top + num) * xres);
	UINT16 *cpy_addr = (UINT16 *)fb_base_addr + ((bottom - num) * xres);

	UINT16 *data = kmalloc(2 * num * xres, 0);
	assert(data);
	//printf("top_addr=%X,new_addr =%X,size=%d,bottom=%d,top=%d,num=%d",top_addr, new_addr,2 * xsize * (bottom - top - num),bottom,top,num);
	assert((bottom - top - num) <= 272);
	memcpy(data, top_addr, 2 * xres * num);
	memmove(top_addr, new_addr, 2 * xres * (bottom - top - num));
	memcpy(cpy_addr, data, 2 * xres * num);

	kfree(data);
}
