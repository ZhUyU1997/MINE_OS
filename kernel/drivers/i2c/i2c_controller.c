#include <sys/types.h>
#include <string.h>
#include "i2c_controller.h"

#define I2C_CONTROLLER_NUM 10

/* 有一个i2c_controller数组用来存放各种不同芯片的操作结构体 */
static struct i2c_controller *i2c_controllers[I2C_CONTROLLER_NUM];
static struct i2c_controller *i2c_con_selected;

void register_i2c_controller(struct i2c_controller *p) {
	int i;
	for (i = 0; i < I2C_CONTROLLER_NUM; i++) {
		if (i2c_controllers[i] == NULL) {
			i2c_controllers[i] = p;
			return;
		}
	}
}

/* 根据名字来选择某款I2C控制器 */
int select_i2c_controller(char *name) {
	int i;
	for (i = 0; i < I2C_CONTROLLER_NUM; i++) {
		if (i2c_controllers[i] && !strcmp(name, i2c_controllers[i]->name)) {
			i2c_con_selected = i2c_controllers[i];
			return 0;
		}
	}
	return -1;
}

/* 实现 i2c_transfer 接口函数 */
int i2c_transfer(struct i2c_msg *msgs, int num) {
	return i2c_con_selected->master_xfer(msgs, num);
}

void i2c_init(void) {
	/* 注册下面的I2C控制器 */
	s3c2440_i2c_con_add();

	/* 选择某款I2C控制器 */
	select_i2c_controller("s3c2440");

	/* 调用它的init函数 */
	i2c_con_selected->init();
}


