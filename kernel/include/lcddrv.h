/*
 * FILE: lcddrv.h
 * 操作LCD控制器、调色板等的底层函数接口
 */

#ifndef __LCDDRV_H__
#define __LCDDRV_H__

#include <sys/types.h>

#define LOWER21BITS(n)  ((n) & 0x1fffff)

#define BPPMODE_1BPP    0x8
#define BPPMODE_2BPP    0x9
#define BPPMODE_4BPP    0xA
#define BPPMODE_8BPP    0xB
#define BPPMODE_16BPP   0xC
#define BPPMODE_24BPP   0xD

#define LCDTYPE_TFT     0x3

#define ENVID_DISABLE   0
#define ENVID_ENABLE    1

#define FORMAT8BPP_5551 0
#define FORMAT8BPP_565  1

#define HSYNC_NORM      0
#define HSYNC_INV       1

#define VSYNC_NORM      0
#define VSYNC_INV       1

#define VDEN_NORM		0
#define VDEN_INV		1

#define BSWP            1
#define HWSWP           1

#define MODE_TFT_8BIT_480272 	 (0x410c)
#define MODE_TFT_16BIT_480272    (0x4110)

#define LCDFRAMEBUFFER 0x33c00000


void Lcd_Port_Init(void);
void Tft_Lcd_Init(int type);
void Lcd_Palette8Bit_Init(void);
void Lcd_EnvidOnOff(int onoff);
void Lcd_PowerEnable(int invpwren, int pwren);
void ClearScrWithTmpPlt(UINT32 color);
void DisableTmpPlt(void);
void ChangePalette(UINT32 color);


#endif /*__LCDDRV_H__*/
