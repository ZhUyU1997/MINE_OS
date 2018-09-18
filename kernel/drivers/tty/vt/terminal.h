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

#define TFTWIDTH  480
#define TFTHEIGHT 272


#ifdef __cplusplus
extern "C" {
#endif

void terminal_init(void);
void drawFastVLine(int16_t x, int16_t y, int16_t h,
						   uint16_t color);
void drawFastHLine(int16_t x, int16_t y, int16_t h,
						   uint16_t color);
void setRotation(uint8_t m) ;
void drawString(uint16_t x, uint16_t y, const char *text);
void drawChar(uint16_t x, uint16_t y, uint8_t c);
void setBackColor(uint16_t col);
void setFrontColor(uint16_t col);
void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
					  uint16_t color);

void setScrollStart(uint16_t start);
void setScrollMargins(uint16_t top, uint16_t bottom);

uint16_t terminal_width(void);
uint16_t terminal_height(void);

#ifdef __cplusplus
}
#endif
