/**
	This file is part of FORTMAX.

	FORTMAX is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	FORTMAX is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FORTMAX.  If not, see <http://www.gnu.org/licenses/>.

	Copyright: Martin K. Schröder (info@fortmax.se) 2014
	Credits: Adafruit for original setup code
*/

#pragma once

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;

#define ILI9340_TFTWIDTH  480
#define ILI9340_TFTHEIGHT 272

/*
#define ILI9340_PWCTR6  0xFC

*/

// Color definitions
#define	ILI9340_BLACK   0x0000
#define	ILI9340_BLUE    0x001F
#define	ILI9340_RED     0xF800
#define	ILI9340_GREEN   0x07E0
#define ILI9340_CYAN    0x07FF
#define ILI9340_MAGENTA 0xF81F


#define ILI9340_YELLOW  0xFFE0
#define ILI9340_WHITE   0xFFFF


#ifdef __cplusplus
extern "C" {
#endif

void ili9340_init(void);
void ili9340_drawFastVLine(int16_t x, int16_t y, int16_t h,
						   uint16_t color);
void ili9340_drawFastHLine(int16_t x, int16_t y, int16_t h,
						   uint16_t color);
void ili9340_setRotation(uint8_t m) ;
void ili9340_drawString(uint16_t x, uint16_t y, const char *text);
void ili9340_drawChar(uint16_t x, uint16_t y, uint8_t c);
void ili9340_setBackColor(uint16_t col);
void ili9340_setFrontColor(uint16_t col);
void ili9340_fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
					  uint16_t color);

void ili9340_setScrollStart(uint16_t start);
void ili9340_setScrollMargins(uint16_t top, uint16_t bottom);

uint16_t ili9340_width(void);
uint16_t ili9340_height(void);

#ifdef __cplusplus
}
#endif
