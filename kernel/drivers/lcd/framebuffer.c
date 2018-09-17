#include "assert.h"
#include "framebuffer.h"
#include "lcd.h"

/* 画点 */
void PutPixel(UINT32 x, UINT32 y, UINT16 color) {
	//TODO:
	if (x < 0 || x >= xsize || y < 0 || y >= ysize) {
		return;
	}
	UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
	*addr = (UINT16) color;
}

UINT16 GetPixel(UINT32 x, UINT32 y) {
	//TODO:
	if (x < 0 || x >= xsize || y < 0 || y >= ysize) {
		return 0;
	}
	UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
	return *addr;
}

void MovLine(UINT32 dis_y, UINT32 src_y, UINT32 num) {
	UINT16 *src_addr = (UINT16 *)fb_base_addr + (src_y * xsize);
	UINT16 *dis_addr = (UINT16 *)fb_base_addr + (dis_y * xsize);
	memmove(dis_addr, src_addr, xsize * num);
}

void ScrollLine(UINT32 top, UINT32 bottom, UINT32 num) {
	if(!num) return;
	UINT16 *top_addr = (UINT16 *)fb_base_addr + (top * xsize);
	UINT16 *new_addr = (UINT16 *)fb_base_addr + ((top + num) * xsize);
	UINT16 *cpy_addr = (UINT16 *)fb_base_addr + ((bottom - num) * xsize);

	UINT16 *data = kmalloc(2 * num * xsize, 0);
	assert(data);
	//printf("top_addr=%X,new_addr =%X,size=%d,bottom=%d,top=%d,num=%d",top_addr, new_addr,2 * xsize * (bottom - top - num),bottom,top,num);
	assert((bottom - top - num) <= 272);
	memcpy(data, top_addr, 2 * xsize * num);
	memmove(top_addr, new_addr, 2 * xsize * (bottom - top - num));
	memcpy(cpy_addr, data, 2 * xsize * num);

	kfree(data);
}
