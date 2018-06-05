#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <sys/types.h>

U16 convert888_565(U32 rgb);
void ClearScr(U32 color);
void DrawLine(U32 x1, U32 y1, U32 x2, U32 y2, U32 color);
void DrawCircle(U32 x, U32 y, U32 r, U32 color);
void DispCross(U32 x, U32 y, U32 color);
void put_font(U32 x, U32 y, U8 c);
void lcd_set_text_color(U32 color);
void lcd_set_background_color(U8 color);
void lcd_putc(U8 c);
void lcd_putstr(U32 x, U32 y, char *str)
#endif
