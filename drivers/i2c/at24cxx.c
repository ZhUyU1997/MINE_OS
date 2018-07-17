#include <timer.h>
#include "i2c_controller.h"

#define AT24CXX_ADDR 0x50

int at24cxx_write(unsigned int addr, unsigned char *data, int len) {
	i2c_msg msg;
	int i;
	int err;
	unsigned char buf[2];


	for (i = 0; i < len; i++) {
		buf[0] = addr++;
		buf[1] = data[i];

		/* 构造i2c_msg */
		msg.addr  = AT24CXX_ADDR;
		msg.flags = 0; /* write */
		msg.len   = 2;
		msg.buf   = buf;
		msg.err   = 0;
		msg.cnt_transferred = -1;

		/* 调用i2c_transfer */
		err = i2c_transfer(&msg, 1);
		udelay(1000);
		if (err)
			return err;
	}

	return 0;
}


int at24cxx_read(unsigned int addr, unsigned char *data, int len) {
	i2c_msg msg[2];
	int err;

	/* 构造i2c_msg */
	msg[0].addr  = AT24CXX_ADDR;
	msg[0].flags  = 0; /* write */
	msg[0].len   = 1;
	msg[0].buf   = &addr;
	msg[0].err   = 0;
	msg[0].cnt_transferred = -1;

	msg[1].addr  = AT24CXX_ADDR;
	msg[1].flags  = 1; /* read */
	msg[1].len   = len;
	msg[1].buf   = data;
	msg[1].err   = 0;
	msg[1].cnt_transferred = -1;

	/* 调用i2c_transfer */
	err = i2c_transfer(&msg, 2);
	if (err)
		return err;
	return 0;
}

