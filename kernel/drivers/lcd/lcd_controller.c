#include <string.h>
#include "lcd_controller.h"

#define LCD_CONTROLLER_NUM 10

static struct lcd_controller *array_lcd_controller[LCD_CONTROLLER_NUM];
static struct lcd_controller *lcd_controller_selected;

int register_lcd_controller(struct lcd_controller *lcdcon) {
	int i;
	for (i = 0; i < LCD_CONTROLLER_NUM; i++) {
		if (!array_lcd_controller[i]) {
			array_lcd_controller[i] = lcdcon;
			return i;
		}
	}
	return -1;
}

int select_lcd_controller(char *name) {
	int i;
	for (i = 0; i < LCD_CONTROLLER_NUM; i++) {
		if (array_lcd_controller[i] && !strcmp(array_lcd_controller[i]->name, name)) {
			lcd_controller_selected = array_lcd_controller[i];
			return i;
		}
	}
	return -1;
}


/* 向上: 接收不同LCD的参数
 * 向下: 使用这些参数设置对应的LCD控制器
 */

int lcd_controller_init(struct lcd_params *lcdparams) {
	/* 调用所选择的LCD控制器的初始化函数 */
	if (lcd_controller_selected) {
		lcd_controller_selected->init(lcdparams);
		lcd_controller_selected->init_palette();
		return 0;
	}
	return -1;
}

void lcd_controller_enable(void) {
	if (lcd_controller_selected) {
		lcd_controller_selected->enable();
	}
}

void lcd_controller_disable(void) {
	if (lcd_controller_selected) {
		lcd_controller_selected->disable();
	}
}


void lcd_contoller_add(void) {
	s3c2440_lcd_contoller_add();
}

