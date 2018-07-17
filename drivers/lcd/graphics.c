#include <ctype.h>
#include <framebuffer.h>
#include "lcd.h"

#define SCREEN_X xsize
#define SCREEN_Y ysize
#define FONT_W 8
#define FONT_H 16

static U32 text_color = 0x0;
static U32 background_color = 0xffffff;

/* rgb: 0x00RRGGBB */
U16 convert888_565(U32 rgb) {
	int r = (rgb >> 16) & 0xff;
	int g = (rgb >> 8) & 0xff;
	int b = rgb & 0xff;

	/* rgb565 */
	r = r >> 3;
	g = g >> 2;
	b = b >> 3;

	return ((r << 11) | (g << 5) | (b));
}

/* 清屏 */
void ClearScr(U32 color) {
	UINT32 x, y;

	color = convert888_565(color);
	for (y = 0; y < ysize; y++)
		for (x = 0; x < xsize; x++)
			PutPixel(x, y, color);
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
void DrawLine(int x1, int y1, int x2, int y2, U32 color) {
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

void DrawCircle(U32 x, U32 y, U32 r, U32 color) {
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

void DrawFillRect(U32 x, U32 y, U32 w, U32 h, U32 color) {
	color = convert888_565(color);
	for(int i = 0; i < h; i++){
		for(int j = 0; j < w; j++){
			PutPixel(x + j, y + i, color);
		}
	}
}

void DispCross(U32 x, U32 y, U32 color) {
	DrawLine(x - 10, y, x + 10, y, color);
	DrawLine(x, y - 10, x, y + 10, color);
}


extern const unsigned char fontdata_8x8[];
extern const unsigned char fontdata_8x16[];
void put_font(U32 x, U32 y, U8 c) {
	unsigned char line_dots;
	/* 获得字模 */
	unsigned char *char_dots = fontdata_8x16 + c * FONT_H;
	U32 text_color_temp = convert888_565(text_color);
	U32 background_color_temp = convert888_565(background_color);

	/* 在framebuffer里描点 */
	for (int i = 0; i < FONT_H; i++) {
		line_dots = char_dots[i];
		for (int j = 0; j < FONT_W; j++) {
			if (line_dots & (0x80 >> j)) {
				PutPixel(x + j,  y + i, text_color_temp);
			} else {
				PutPixel(x + j,  y + i, background_color_temp);
			}
		}
	}
}


static int x = 0;
static int y = 0;

static void set_pos(U32 pos_x, U32 pos_y){
	x = pos_x;
	y = pos_y;
}

void set_text_color(U32 frcolor, U32 bgcolor) {
	text_color = frcolor;
	background_color = bgcolor;
}

void lcd_putc(U8 c) {
	if (isgraph(c)) {
		put_font(x, y, c);
		x = (x + FONT_W) % SCREEN_X;
		if (x == 0) {
			y = (y + FONT_H) % SCREEN_Y;
		}
	} else if (iscntrl(c) || isspace(c)) {
		switch (c) {
			case '\r':
				x = 0;
				break;
			case '\b':
				//if(x<=0) x=480-8;
				//else x = (x - 8)%480;
				//x=(480+(x/8-1)*8)%480;
				//x=(480+x-8)%480;
				//x=(472+x)%480;
				//TODO:跨屏问题，暂不支持滚屏
				if (x == 0) {
					y = (y + SCREEN_Y - FONT_H) % SCREEN_Y;
				}
				x = (SCREEN_X + x) % SCREEN_X;
				put_font(x, y, ' ');
				break;
			case '\t':
				for (int i = 0; i < 4 - (x % FONT_W) % 4; i++) {
					put_font(x, y, ' ');
					x = (x + FONT_W) % SCREEN_X;
					if (x == 0) {
						y = (y + FONT_H) % SCREEN_Y;
						break;
					}
				}
				break;
			case '\n':
				x = 0;
				y = (y + FONT_H) % SCREEN_Y;
				break;
			case ' ':
				put_font(x, y, ' ');
				x = (x + FONT_W) % SCREEN_X;
				if (x == 0) {
					y = (y + FONT_H) % SCREEN_Y;
				}
				break;
			default:
				break;
		}
	}
}

void lcd_putstr(U32 x, U32 y, char *str){
	set_pos(x, y);
	for(U32 i = 0; i < strlen(str); i++){
		lcd_putc(str[i]);
	}
}
