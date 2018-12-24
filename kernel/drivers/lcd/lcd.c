#include <string.h>
#include "lcd.h"
#include "lcd_controller.h"

#define LCD_NUM 10

static struct lcd_params *array_lcd[LCD_NUM];
static struct lcd_params *lcd_selected;

int register_lcd(struct lcd_params *lcd) {
	int i;
	for (i = 0; i < LCD_NUM; i++) {
		if (!array_lcd[i]) {
			array_lcd[i] = lcd;
			return i;
		}
	}
	return -1;
}

int select_lcd(char *name) {
	int i;
	for (i = 0; i < LCD_NUM; i++) {
		if (array_lcd[i] && !strcmp(array_lcd[i]->name, name)) {
			lcd_selected = array_lcd[i];
			return i;
		}
	}
	return -1;
}

void get_lcd_params(unsigned int *fb_base, int *xres, int *yres, int *bpp) {
	*fb_base = lcd_selected->fb_base;
	*xres = lcd_selected->xres;
	*yres = lcd_selected->yres;
	*bpp = lcd_selected->bpp;
}

void lcd_enable(void) {
	lcd_controller_enable();
}

void lcd_disable(void) {
	lcd_controller_disable();
}

int lcd_init(void) {
	/* 注册LCD */
	lcd_4_3_add();

	/* 注册LCD控制器 */
	lcd_contoller_add();

	/* 选择某款LCD */
	select_lcd("lcd_4.3");

	/* 选择某款LCD控制器 */
	select_lcd_controller("s3c2440");

	/* 使用LCD的参数, 初始化LCD控制器 */
	lcd_controller_init(lcd_selected);
	return 0;
}

