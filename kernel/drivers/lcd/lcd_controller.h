#ifndef _LCD_CONTROLLER_H
#define _LCD_CONTROLLER_H

#include "lcd.h"

struct lcd_controller {
	char *name;
	void (*init)(struct lcd_params *lcdparams);
	void (*enable)(void);
	void (*disable)(void);
	void (*init_palette)(void);
};

int register_lcd_controller(struct lcd_controller *lcdcon);
int select_lcd_controller(char *name);
int lcd_controller_init(struct lcd_params *lcdparams);
void lcd_controller_enable(void);
void lcd_controller_disable(void);
void lcd_contoller_add(void);

#endif /* _LCD_CONTROLLER_H */

