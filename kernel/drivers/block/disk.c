/***************************************************
*		版权声明
*
*	本操作系统名为：MINE
*	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
*	只允许个人学习以及公开交流使用
*
*	代码最终所有权及解释权归田宇所有；
*
*	本模块作者：	田宇
*	EMail:		345538255@qq.com
*
*
***************************************************/

#include "disk.h"
#include "memory.h"
#include "printk.h"
#include "lib.h"
#include "block.h"
#include "semaphore.h"


long IDE_open() {
	color_printk(BLACK, WHITE, "DISK1 Opened\n");
	return 1;
}

long IDE_close() {
	color_printk(BLACK, WHITE, "DISK1 Closed\n");
	return 1;
}

long IDE_ioctl(long cmd, long arg) {
	return 0;
}

long IDE_transfer(long cmd, unsigned long blocks, long count, unsigned char * buffer) {
	if (cmd == ATA_READ_CMD) {
		return Read_Block(blocks, count, buffer);
	} else if (cmd == ATA_WRITE_CMD) {
		return Write_Block(blocks, count, buffer);
	}

	return 0;
}

struct block_device_operation IDE_device_operation = {
	.open = IDE_open,
	.close = IDE_close,
	.ioctl = IDE_ioctl,
	.transfer = IDE_transfer,
};

void disk_init() {
}

void disk_exit() {
}
