
#ifndef _LCD_CONTROLLER_H
#define _LCD_CONTROLLER_H

#include "lcd.h"

typedef struct lcd_controller {
	char *name;
	void (*init)(p_lcd_params plcdparams);
	void (*enable)(void);
	void (*disable)(void);
	void (*init_palette)(void);
}lcd_controller, *p_lcd_controller;

#endif /* _LCD_CONTROLLER_H */

