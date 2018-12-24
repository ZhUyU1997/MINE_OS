#include <assert.h>
#include <graphics.h>
#include <memory.h>
#include "framebuffer.h"
#include "lcd.h"

/* 获得LCD参数 */
static unsigned int fb_base_addr;
static int xres, yres, bpp;

void fb_get_lcd_params(void) {
	get_lcd_params(&fb_base_addr, &xres, &yres, &bpp);
}

/* 画点 */
void PutPixel(u32_t x, u32_t y, u16_t color) {
	//TODO:
	if (x < 0 || x >= xres || y < 0 || y >= yres) {
		return;
	}
	u16_t *addr = (u16_t *)fb_base_addr + (y * xres + x);
	*addr = (u16_t) color;
}

u16_t GetPixel(u32_t x, u32_t y) {
	//TODO:
	if (x < 0 || x >= xres || y < 0 || y >= yres) {
		return 0;
	}
	u16_t *addr = (u16_t *)fb_base_addr + (y * xres + x);
	return *addr;
}

/* 清屏 */
void ClearScreen(u32_t color) {
	u32_t x, y;

	color = convert888_565(color);
	for (y = 0; y < yres; y++)
		for (x = 0; x < xres; x++)
			PutPixel(x, y, color);
}

void MovLine(u32_t dis_y, u32_t src_y, u32_t num) {
	u16_t *src_addr = (u16_t *)fb_base_addr + (src_y * xres);
	u16_t *dis_addr = (u16_t *)fb_base_addr + (dis_y * xres);
	memmove(dis_addr, src_addr, xres * num);
}

void ScrollLine(u32_t top, u32_t bottom, u32_t num) {
	if (!num) return;
	u16_t *top_addr = (u16_t *)fb_base_addr + (top * xres);
	u16_t *new_addr = (u16_t *)fb_base_addr + ((top + num) * xres);
	u16_t *cpy_addr = (u16_t *)fb_base_addr + ((bottom - num) * xres);

	u16_t *data = kmalloc(2 * num * xres, 0);
	assert(data);
	//printf("top_addr=%X,new_addr =%X,size=%d,bottom=%d,top=%d,num=%d",top_addr, new_addr,2 * xsize * (bottom - top - num),bottom,top,num);
	assert((bottom - top - num) <= 272);
	memcpy(data, top_addr, 2 * xres * num);
	memmove(top_addr, new_addr, 2 * xres * (bottom - top - num));
	memcpy(cpy_addr, data, 2 * xres * num);

	kfree(data);
}
