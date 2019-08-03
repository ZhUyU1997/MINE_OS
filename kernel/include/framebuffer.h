#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <types.h>

void PutPixel(UINT32 x, UINT32 y, UINT16 color);
UINT16 GetPixel(UINT32 x, UINT32 y);
void ClearScreen(u32_t color);
void MovLine(UINT32 dis_y, UINT32 src_y, UINT32 num);
void ScrollLine(UINT32 top, UINT32 bottom, UINT32 num);
void DispCross(u32_t x, u32_t y, u32_t color);

#endif /*__FRAMEBUFFER_H__*/
