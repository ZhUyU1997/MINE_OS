#define PAGE_DATA_SIZE  (2048)
#define PAGE_SPARE_SIZE (64)
#define PAGES_PER_BLOCK (64)
#define PAGE_SHIFT (11)
#define BLOCK_SHIFT (17)
#define PAGE_TO_BLOCK_SHIFT (6)
#define BLOCK_TO_PAGE_SHIFT (6)
#define PAGE_TO_BLOCK(page) ((page)>>PAGE_TO_BLOCK_SHIFT)
#define BLOCK_TO_PAGE(block) ((block)<<BLOCK_TO_PAGE_SHIFT)

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

void nand_init_ll(void) {
#define TACLS   0
#define TWRPH0  1
#define TWRPH1  0
	/* 设置时序 */
	NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	/* 使能NAND Flash控制器, 初始化ECC, 禁止片选 */
	NFCONT = (1<<4)|(1<<1)|(1<<0);
	nand_reset();
}

static void nand_select(void) {
	NFCONT &= ~(1<<1);
}

static void nand_deselect(void) {
	NFCONT |= (1<<1);
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

/* 复位 */
static void nand_reset(void) {
	nand_select();
	nand_cmd(CMD_RESET);  // 复位命令
	nand_wait_ready();
	nand_deselect();
}

static int nand_is_bad_block(unsigned int block_number) {
	unsigned int col  = 2048;
	unsigned int page = BLOCK_TO_PAGE(block_number);
	unsigned char val1,val2;

	
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

	if ((val1 == 0xff)&&(val2 == 0xff))
		return 0;  
	else
		return 1;/* bad blcok */
}


void nand_read_ll(unsigned char *buf,unsigned int addr , unsigned int len) {
	int col ;
	int i = 0;

	while (i < len) {
		/* 一个block只判断一次 */
		if (!(addr & 0x1FFFF)) { //读到新的block
			if(nand_is_bad_block(addr)){
				addr += (128*1024);  /* 跳过当前block */
				continue;
			}
		}

		nand_select();			/* 1. 选中 */
		nand_cmd(CMD_READ1);	/* 2. 发出读命令00h */
		nand_addr(addr);		/* 3. 发出地址(分5步发出) */
		nand_cmd(CMD_READ2);	/* 4. 发出读命令30h */
		nand_wait_ready();		/* 5. 判断状态 */

		/* 6. 读数据 */
		for (col=0 ; (col < 2048) && (i < len); col++) {
			buf[i] = nand_read_data();
			i++;
			addr++;
		}
		
		nand_deselect();/* 7. 取消选中 */

	}
}