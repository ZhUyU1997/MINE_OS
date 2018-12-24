#include <ctype.h>
#include <framebuffer.h>
#include "lcd.h"

#define SCREEN_X xsize
#define SCREEN_Y ysize
#define FONT_W 8
#define FONT_H 16

static u32_t text_color = 0x0;
static u32_t background_color = 0xffffff;

/* rgb: 0x00RRGGBB */
u16_t convert888_565(u32_t rgb) {
	int r = (rgb >> 16) & 0xff;
	int g = (rgb >> 8) & 0xff;
	int b = rgb & 0xff;

	/* rgb565 */
	r = r >> 3;
	g = g >> 2;
	b = b >> 3;

	return ((r << 11) | (g << 5) | (b));
}

/*
 * 画线
 * 输入参数：
 *     x1、y1 : 起点坐标
 *     x2、y2 : 终点坐标
 *     color  : 颜色值
 *         对于16BPP: color的格式为0xAARRGGBB (AA = 透明度),
 *     需要转换为5:6:5格式
 *         对于8BPP: color为调色板中的索引值，
 *     其颜色取决于调色板中的数值
 */
void DrawLine(int x1, int y1, int x2, int y2, u32_t color) {
	int dx, dy, e;
	dx = x2 - x1;
	dy = y2 - y1;

	if (dx >= 0) {
		if (dy >= 0) { // dy>=0
			if (dx >= dy) { // 1/8 octant
				e = dy - dx / 2;
				while (x1 <= x2) {
					PutPixel(x1, y1, convert888_565(color));
					if (e > 0) {
						y1 += 1;
						e -= dx;
					}
					x1 += 1;
					e += dy;
				}
			} else {    // 2/8 octant
				e = dx - dy / 2;
				while (y1 <= y2) {
					PutPixel(x1, y1, convert888_565(color));
					if (e > 0) {
						x1 += 1;
						e -= dy;
					}
					y1 += 1;
					e += dx;
				}
			}
		} else {       // dy<0
			dy = -dy; // dy=abs(dy)

			if (dx >= dy) { // 8/8 octant
				e = dy - dx / 2;
				while (x1 <= x2) {
					PutPixel(x1, y1, convert888_565(color));
					if (e > 0) {
						y1 -= 1;
						e -= dx;
					}
					x1 += 1;
					e += dy;
				}
			} else {    // 7/8 octant
				e = dx - dy / 2;
				while (y1 >= y2) {
					PutPixel(x1, y1, convert888_565(color));
					if (e > 0) {
						x1 += 1;
						e -= dy;
					}
					y1 -= 1;
					e += dx;
				}
			}
		}
	} else { //dx<0
		dx = -dx;   //dx=abs(dx)
		if (dy >= 0) { // dy>=0
			if (dx >= dy) { // 4/8 octant
				e = dy - dx / 2;
				while (x1 >= x2) {
					PutPixel(x1, y1, convert888_565(color));
					if (e > 0) {
						y1 += 1;
						e -= dx;
					}
					x1 -= 1;
					e += dy;
				}
			} else {    // 3/8 octant
				e = dx - dy / 2;
				while (y1 <= y2) {
					PutPixel(x1, y1, convert888_565(color));
					if (e > 0) {
						x1 -= 1;
						e -= dy;
					}
					y1 += 1;
					e += dx;
				}
			}
		} else {       // dy<0
			dy = -dy; // dy=abs(dy)

			if (dx >= dy) { // 5/8 octant
				e = dy - dx / 2;
				while (x1 >= x2) {
					PutPixel(x1, y1, convert888_565(color));
					if (e > 0) {
						y1 -= 1;
						e -= dx;
					}
					x1 -= 1;
					e += dy;
				}
			} else {    // 6/8 octant
				e = dx - dy / 2;
				while (y1 >= y2) {
					PutPixel(x1, y1, convert888_565(color));
					if (e > 0) {
						x1 -= 1;
						e -= dy;
					}
					y1 -= 1;
					e += dx;
				}
			}
		}
	}
}

void DrawCircle(u32_t x, u32_t y, u32_t r, u32_t color) {
	int num;
	int a = 0;
	int b = r;
	color = convert888_565(color);
	while (22 * b * b >= r * r) {	 // 1/8圆即可
		PutPixel(x + a, y - b, color); // 0~1
		PutPixel(x - a, y - b, color); // 0~7
		PutPixel(x - a, y + b, color); // 4~5
		PutPixel(x + a, y + b, color); // 4~3

		PutPixel(x + b, y + a, color); // 2~3
		PutPixel(x + b, y - a, color); // 2~1
		PutPixel(x - b, y - a, color); // 6~7
		PutPixel(x - b, y + a, color); // 6~5

		a++;
		num = (a * a + b * b) - r * r;
		if (num > 0) {
			b--;
			a--;
		}
	}
}

void DrawFillRect(u32_t x, u32_t y, u32_t w, u32_t h, u32_t color) {
	color = convert888_565(color);
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			PutPixel(x + j, y + i, color);
		}
	}
}

void DispCross(u32_t x, u32_t y, u32_t color) {
	DrawLine(x - 10, y, x + 10, y, color);
	DrawLine(x, y - 10, x, y + 10, color);
}
