
#ifndef _I2C_CONTROLLER_H
#define _I2C_CONTROLLER_H

struct i2c_msg {
	unsigned int addr;  /* 7bits */
	int flags;  /* 0 - write, 1 - read */
	int len;
	int cnt_transferred;
	int err;
	unsigned char *buf;
};

struct i2c_controller {
	int (*init)(void);
	int (*master_xfer)(struct i2c_msg *msgs, int num);
	char *name;
};

void register_i2c_controller(struct i2c_controller *p);

#endif /* _I2C_CONTROLLER_H */


