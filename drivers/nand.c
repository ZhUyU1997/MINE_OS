#include <timer.h>
#include "nand.h"
/* NAND FLASH控制器 */
#define NFCONF (*((volatile unsigned long *)0x4E000000))
#define NFCONT (*((volatile unsigned long *)0x4E000004))
#define NFCMMD (*((volatile unsigned char *)0x4E000008))
#define NFADDR (*((volatile unsigned char *)0x4E00000C))
#define NFDATA (*((volatile unsigned char *)0x4E000010))
#define NFSTAT (*((volatile unsigned char *)0x4E000020))

#define CMD_READ1	0x00				//页读命令周期1
#define CMD_READ2	0x30				//页读命令周期2
#define CMD_READID	0x90				//读ID命令
#define CMD_WRITE1	0x80				//页写命令周期1
#define CMD_WRITE2	0x10				//页写命令周期2
#define CMD_ERASE1	0x60				//块擦除命令周期1
#define CMD_ERASE2	0xd0				//块擦除命令周期2
#define CMD_STATUS	0x70				//读状态命令
#define CMD_RESET	0xff				//复位
#define CMD_RANDOMREAD1	0x05			//随意读命令周期1
#define CMD_RANDOMREAD2	0xE0			//随意读命令周期2
#define CMD_RANDOMWRITE	0x85			//随意写命令

static void nand_reset(void);

void nand_init(void) {
#define TACLS   0
#define TWRPH0  1
#define TWRPH1  0
	/* 设置时序 */
	NFCONF = (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4);
	/* 使能NAND Flash控制器, 初始化ECC, 禁止片选 */
	NFCONT = (1 << 4) | (1 << 1) | (1 << 0);
	nand_reset();
}

static void nand_select(void) {
	NFCONT &= ~(1 << 1);
}

static void nand_deselect(void) {
	NFCONT |= (1 << 1);
}

static void nand_cmd(unsigned char cmd) {
	volatile int i;
	NFCMMD = cmd;
	for (i = 0; i < 10; i++);
}

static void nand_addr(unsigned int addr) {
	unsigned int col  = addr % 2048;
	unsigned int page = addr / 2048;
	volatile int i;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);
}

static void nand_page(unsigned int page) {
	volatile int i;

	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);
}

static void nand_col(unsigned int col) {
	volatile int i;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
}

static void nand_wait_ready(void) {
	while (!(NFSTAT & 1));
}

static unsigned char nand_read_data(void) {
	return NFDATA;
}

static void nand_write_data(unsigned char data) {
	NFDATA = data;
}

/* 复位 */
static void nand_reset(void) {
	nand_select();
	nand_cmd(CMD_RESET);  // 复位命令
	nand_wait_ready();
	nand_deselect();
}

int nand_is_bad_block(unsigned int block_number) {
	unsigned int col  = 2048;
	unsigned int page = BLOCK_TO_PAGE(block_number);
	unsigned char val1, val2;


	nand_select();		/* 1. 选中 */
	nand_cmd(CMD_READ1);		/* 2. 发出读命令00h */

	/* 3. 发出地址(分5步发出) */
	nand_col(col);
	nand_page(page);


	nand_cmd(CMD_READ2);		/* 4. 发出读命令30h */
	nand_wait_ready();/* 5. 判断状态 */

	/* 6. 读数据 */
	val1 = nand_read_data();
	val2 = nand_read_data();
	/* 7. 取消选中 */
	nand_deselect();

	if ((val1 == 0xff) && (val2 == 0xff))
		return 0;
	else
		return 1;/* bad blcok */
}


void nand_read(unsigned char *buf, unsigned int addr , unsigned int len) {
	int col ;
	int i = 0;

	while (i < len) {
		/* 一个block只判断一次 */
		if (!(addr & 0x1FFFF)) { //读到新的block
			if (nand_is_bad_block(addr)) {
				addr += (128 * 1024); /* 跳过当前block */
				continue;
			}
		}

		nand_select();			/* 1. 选中 */
		nand_cmd(CMD_READ1);	/* 2. 发出读命令00h */
		nand_addr(addr);		/* 3. 发出地址(分5步发出) */
		nand_cmd(CMD_READ2);	/* 4. 发出读命令30h */
		nand_wait_ready();		/* 5. 判断状态 */

		/* 6. 读数据 */
		for (col = 0 ; (col < 2048) && (i < len); col++) {
			buf[i] = nand_read_data();
			i++;
			addr++;
		}

		nand_deselect();/* 7. 取消选中 */

	}
}

int nand_read_with_oob(unsigned int page, unsigned char *buf, unsigned int buf_len, unsigned char *oob, unsigned int oob_len) {
	int i;
	nand_select();
	nand_cmd(CMD_READ1);
	nand_addr(page << PAGE_SHIFT);
	nand_cmd(CMD_READ2);
	nand_wait_ready();
	//TODO:优化,随机读
	if (buf) {
		for (i = 0 ; i < 2048; i++) {
			if (i < buf_len)
				buf[i] = nand_read_data();
			else
				nand_read_data();
		}
	} else {
		for (i = 0 ; i < 2048; i++) {
			nand_read_data();
		}
	}

	nand_read_data();
	nand_read_data();

	for (i = 0; i < 16; i++) {
		nand_read_data();
	}

	nand_read_data();
	nand_read_data();

	if (oob) {
		for (i = 0 ; i < oob_len; i++) {
			oob[i] = nand_read_data();
		}
	}
	nand_deselect();
	return 1;
}

int nand_read_page(unsigned int page, unsigned char *buf, unsigned int buf_len, unsigned char *spare, unsigned int spare_len) {
	int i;
	nand_select();
	nand_cmd(CMD_READ1);
	nand_addr(page << PAGE_SHIFT);
	nand_cmd(CMD_READ2);
	nand_wait_ready();
	//TODO:优化,随机读
	if (buf) {
		for (i = 0 ; i < 2048; i++) {
			if (i < buf_len)
				buf[i] = nand_read_data();
			else
				nand_read_data();
		}
	} else {
		for (i = 0 ; i < 2048; i++) {
			nand_read_data();
		}
	}

	if (spare) {
		for (i = 0 ; i < spare_len; i++) {
			spare[i] = nand_read_data();
		}
	}
	nand_deselect();
	return 1;
}

int nand_write(unsigned char *buf, unsigned int addr , unsigned int len) {
	int i;
	unsigned char stat;

	int temp = nand_is_bad_block(addr);	//判断该块是否为坏块
	if (temp)
		return -1;				//是坏块，返回

	nand_select();				//打开nandflash片选
	nand_cmd(CMD_WRITE1);		//页写命令周期1
	nand_addr(addr);

	//写入一页数据
	for (i = 0; (i < 2048) && (i < len); i++) {
		nand_write_data(buf[i]);
	}

	nand_cmd(CMD_WRITE2);		//页写命令周期2
	udelay(1000);				//延时一段时间，以等待写操作完成
	nand_cmd(CMD_STATUS);		//读状态命令

	//判断状态值的第6位是否为1，即是否在忙，该语句的作用与NF_DETECT_RB();相同
	do {
		stat = nand_read_data();
	} while (!(stat & 0x40));

	nand_deselect();			//关闭nandflash片选

	if (stat & 0x1) {
		return -2;				//写操作失败
	} else
		return 1;				//写操作成功
}

int nand_write_with_oob(unsigned int page, unsigned char *buf, unsigned int buf_len, unsigned char *oob, unsigned int oob_len) {
	int i;
	unsigned char stat;

	nand_select();				//打开nandflash片选
	nand_cmd(CMD_WRITE1);		//页写命令周期1
	nand_addr(page << PAGE_SHIFT);

	//写入一页数据
	if (buf) {
		for (i = 0; i < buf_len; i++) {
			nand_write_data(buf[i]);
		}
	}
	if (buf_len < 2048) {
		//调整到oob区
		int col = 2048;
		nand_cmd(CMD_RANDOMWRITE); // 页内随机写命令
		nand_col(col);
	}
	nand_write_data(0xff);
	nand_write_data(0xff);

	for (i = 0; i < 16; i++) {
		nand_write_data(0xff);
	}
	nand_write_data(0xff);
	nand_write_data(0xff);

	if (oob) {
		for (i = 0; i < oob_len; i++) {
			nand_write_data(oob[i]);
		}
	}

	nand_cmd(CMD_WRITE2);		//页写命令周期2
	udelay(1000);				//延时一段时间，以等待写操作完成
	nand_cmd(CMD_STATUS);		//读状态命令

	//判断状态值的第6位是否为1，即是否在忙，该语句的作用与NF_DETECT_RB();相同
	do {
		stat = nand_read_data();
	} while (!(stat & 0x40));

	nand_deselect();			//关闭nandflash片选

	if (stat & 0x1) {
		return -2;				//写操作失败
	} else
		return 1;				//写操作成功
}

int nand_write_page(unsigned int page, unsigned char *buf, unsigned int buf_len, unsigned char *spare, unsigned int spare_len) {
	int i;
	unsigned char stat;

	nand_select();				//打开nandflash片选
	nand_cmd(CMD_WRITE1);		//页写命令周期1
	nand_addr(page << PAGE_SHIFT);

	//写入一页数据
	if (buf) {
		for (i = 0; i < buf_len; i++) {
			nand_write_data(buf[i]);
		}
	}
	if (buf_len < 2048) {
		//调整到oob区
		int col = 2048;
		nand_cmd(CMD_RANDOMWRITE); // 页内随机写命令
		nand_col(col);
	}

	if (spare) {
		for (i = 0; i < spare_len; i++) {
			nand_write_data(spare[i]);
		}
	}

	nand_cmd(CMD_WRITE2);		//页写命令周期2
	udelay(1000);				//延时一段时间，以等待写操作完成
	nand_cmd(CMD_STATUS);		//读状态命令

	//判断状态值的第6位是否为1，即是否在忙，该语句的作用与NF_DETECT_RB();相同
	do {
		stat = nand_read_data();
	} while (!(stat & 0x40));

	nand_deselect();			//关闭nandflash片选

	if (stat & 0x1) {
		return -2;				//写操作失败
	} else
		return 1;				//写操作成功
}
int nand_mark_bad_block(unsigned int block_number) {
	unsigned char oob[2] = {0xff, 0xff};
	// 每个block第一页spare区0, 1字节非0xff标记为好坏
	return nand_write_page(BLOCK_TO_PAGE(block_number), 0, 2048, oob, 2);
}

int nand_erase_block(unsigned int block_number) {
	unsigned char stat;

	nand_select();					//打开片选
	nand_cmd(CMD_ERASE1);			//擦除命令周期1
	//写入3个地址周期，从A18开始写起
	nand_page(block_number << 6);
	nand_cmd(CMD_ERASE2);			//擦除命令周期2
	udelay(10000);					//延时一段时间
	nand_cmd(CMD_STATUS);			//读状态命令

	//判断状态值的第6位是否为1，即是否在忙，该语句的作用与NF_DETECT_RB();相同
	do {
		stat = nand_read_data();
	} while (!(stat & 0x40));

	nand_deselect();				//关闭nandflash片选

	//判断状态值的第0位是否为0，为0则擦除操作正确，否则错误
	if (stat & 0x1) {
		return -2;					//擦除操作失败
	} else
		return 1;					//擦除操作成功
}

void nand_print(unsigned int page) {
	char buf[2048] = {0};
	char spare[64] = {0};
	nand_read_page(page, buf, 2048, spare, 64);
	printf("NAND BLOCK:%d PAGE:%d :\n\r", PAGE_TO_BLOCK(page), (page) & 0x3f);
	printf("MAIN DATA:\n\r");
	for (int i = 0; i < 2048; i++) {
		printf("%02X ", buf[i] & 0xff);
		if (i % 16 == 15)
			printf("\n\r");
	}
	printf("SPARE DATA:\n\r");
	for (int i = 0; i < 64; i++) {
		printf("%02X ", spare[i] & 0xff);
		if (i % 16 == 15)
			printf("\n\r");
	}

}

