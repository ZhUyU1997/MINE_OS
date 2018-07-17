#include "framebuffer.h"
#include "lcd.h"

/* 画点 */
void PutPixel(UINT32 x, UINT32 y, UINT16 color) {
	//TODO:
	if (x < 0 || x > xsize - 1 || y < 0 || y > ysize - 1) {
		return;
	}
	UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
	*addr = (UINT16) color;
}

UINT16 GetPixel(UINT32 x, UINT32 y) {
	//TODO:
	if (x < 0 || x > xsize - 1 || y < 0 || y > ysize - 1) {
		return 0;
	}
	UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
	return *addr;
}

