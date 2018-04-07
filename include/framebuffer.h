/*
 * FILE: framebuffer.h
 * 在framebuffer上画点、画线、画同心圆、清屏的函数接口
 */

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <sys/types.h>

/* 
 * 画点
 * 输入参数：
 *     x、y : 象素坐标
 *     color: 颜色值
 *         对于16BPP: color的格式为0xAARRGGBB (AA = 透明度),
 *     需要转换为5:6:5格式
 *         对于8BPP: color为调色板中的索引值，
 *     其颜色取决于调色板中的数值
 */
void PutPixel(UINT32 x, UINT32 y, UINT16 color);
UINT16 GetPixel(UINT32 x, UINT32 y);
/* 
 * 将屏幕清成单色
 * 输入参数：
 *     color: 颜色值
 *         对于16BPP: color的格式为0xAARRGGBB (AA = 透明度),
 *     需要转换为5:6:5格式
 *         对于8BPP: color为调色板中的索引值，
 *     其颜色取决于调色板中的数值
 */
void ClearScr(UINT16 color);
void lcd_set_text_color(unsigned int color);
void lcd_set_background_color(unsigned int color);
#endif /*__FRAMEBUFFER_H__*/
