#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <types.h>

u16_t convert888_565(u32_t rgb);
void ClearScr(u32_t color);
void DrawLine(u32_t x1, u32_t y1, u32_t x2, u32_t y2, u32_t color);
void DrawCircle(u32_t x, u32_t y, u32_t r, u32_t color);
void DispCross(u32_t x, u32_t y, u32_t color);

#endif
