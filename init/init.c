/*
 * init.c: 进行一些初始化
 */ 

#include "s3c24xx.h"
 
void disable_watch_dog(void);
void clock_init(void);
void memsetup(void);
void copy_steppingstone_to_sdram(void);
void clean_bss(void);

/*
 * 关闭WATCHDOG，否则CPU会不断重启
 */
void disable_watch_dog(void)
{
    WTCON = 0;  // 关闭WATCHDOG很简单，往这个寄存器写0即可
}

#define FCLK        200000000
#define HCLK        100000000
#define PCLK        50000000
#define S3C2410_MPLL_200MHZ     ((0x5c<<12)|(0x04<<4)|(0x00))
#define S3C2440_MPLL_200MHZ     ((0x5c<<12)|(0x01<<4)|(0x02))
#define S3C2440_MPLL_400MHZ     ((0x5c<<12)|(0x01<<4)|(0x01))
#define S3C2440_UPLL_48MHZ      ((0x38<<12)|(0x02<<4)|(0x02))
#define S3C2440_UPLL_96MHZ      ((0x38<<12)|(0x02<<4)|(0x01))
/*
 * 对于MPLLCON寄存器，[19:12]为MDIV，[9:4]为PDIV，[1:0]为SDIV
 * 有如下计算公式：
 *  S3C2410: MPLL(FCLK) = (m * Fin)/(p * 2^s)
 *  S3C2410: MPLL(FCLK) = (2 * m * Fin)/(p * 2^s)
 *  其中: m = MDIV + 8, p = PDIV + 2, s = SDIV
 * 对于本开发板，Fin = 12MHz
 * 设置CLKDIVN，令分频比为：FCLK:HCLK:PCLK=1:4:8，
 * FCLK=400MHz,HCLK=100MHz,PCLK=50MHz
 */
void clock_init(void)
{
    // LOCKTIME = 0x00ffffff;   // 使用默认值即可
    //CLKDIVN  = 0x03;            // FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1
	CLKDIVN  = 0x05;            // FCLK:HCLK:PCLK=1:4:8
    /* 如果HDIVN非0，CPU的总线模式应该从“fast bus mode”变为“asynchronous bus mode” */
	__asm__ volatile (
		"mrc    p15, 0, r1, c1, c0, 0\n"        /* 读出控制寄存器 */ 
		"orr    r1, r1, #0xc0000000\n"          /* 设置为“asynchronous bus mode” */
		"mcr    p15, 0, r1, c1, c0, 0\n"        /* 写入控制寄存器 */
		:::"r1"
    );
	/*
	当你同时设置 MPLL 和 UPLL 的值时，你必须首先设置 UPLL 值再设置 MPLL 值。（大约需要 7 个 NOP 的间隔）
	*/
	//UPLLCON = S3C2440_UPLL_48MHZ;
	
	MPLLCON = S3C2440_MPLL_400MHZ;  /* 现在，FCLK=400MHz,HCLK=100MHz,PCLK=50MHz */    
}
/*
 * 启动ICACHE
 */
void enable_ICACNE(void)
{
    __asm__ volatile (
		"mrc    p15, 0, r0, c1, c0, 0\n"		/* 读出控制寄存器 */ 
		"orr    r0, r0, #(1<<12)\n"
		"mcr    p15, 0, r0, c1, c0, 0\n"	/* 写入控制寄存器 */
		:::"r0"
    );
}
/*
 * 设置存储控制器以使用SDRAM
 */
void memsetup(void)
{
    volatile unsigned long *p = (volatile unsigned long *)MEM_CTL_BASE;

    /* 这个函数之所以这样赋值，而不是像前面的实验(比如mmu实验)那样将配置值
     * 写在数组中，是因为要生成”位置无关的代码”，使得这个函数可以在被复制到
     * SDRAM之前就可以在steppingstone中运行
     */
    /* 存储控制器13个寄存器的值 */
    p[0] = 0x22011110;     //BWSCON
    p[1] = 0x00000700;     //BANKCON0
    p[2] = 0x00000700;     //BANKCON1
    p[3] = 0x00000700;     //BANKCON2
    p[4] = 0x00000700;     //BANKCON3  
    p[5] = 0x00000700;     //BANKCON4
    p[6] = 0x00000700;     //BANKCON5
    p[7] = 0x00018005;     //BANKCON6
    p[8] = 0x00018005;     //BANKCON7
    
                                    /* REFRESH,
                                     * HCLK=12MHz:  0x008C07A3,
                                     * HCLK=100MHz: 0x008C04F4
                                     */ 
    p[9]  = 0x008C04F4;
    p[10] = 0x000000B1;     //BANKSIZE
    p[11] = 0x00000030;     //MRSRB6
    p[12] = 0x00000030;     //MRSRB7
}

void copy_steppingstone_to_sdram(void)
{
    unsigned int *pdwSrc  = (unsigned int *)0;
    unsigned int *pdwDest = (unsigned int *)0x30000000;
    
    while (pdwSrc < (unsigned int *)4096)
    {
        *pdwDest = *pdwSrc;
        pdwDest++;
        pdwSrc++;
    }
}

void clean_bss(void)
{
    extern int __bss_start, __bss_end;
    int *p = &__bss_start;
    
    for (; p < &__bss_end; p++)
        *p = 0;
}

int is_boot_from_nor_flash(void) {
	volatile int *p = (volatile int *)0;
	int val;

	val = *p;
	*p = 0x12345678;
	if (*p == 0x12345678) {
		/* 写成功, 是nand启动 */
		*p = val;
		return 0;
	} else {
		/* NOR不能像内存一样写 */
		return 1;
	}
}

int copy_code_to_sdram(unsigned char *buf,unsigned int *addr , unsigned int len)
{
    extern void nand_read_ll(unsigned char *buf,unsigned int addr , unsigned int len);
	int i = 0;

	/* 如果是NOR启动 */
	if (is_boot_from_nor_flash()) {
		while (i < len) {
			buf[i] = addr[i];
			i++;
		}
	} else {
		//nand_init();
		nand_read_ll(buf, (unsigned int)addr, len);
	}
    return 0;
}