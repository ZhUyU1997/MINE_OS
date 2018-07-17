#include <global_config.h>
#include <assert.h>
#define MMU_FULL_ACCESS     (3)			/* 访问权限 */
#define MMU_DOMAIN(x)       (x)			/* 属于哪个域 */
#define MMU_SPECIAL         (1)			/* 必须是1 */
#define MMU_CACHE_ENABLE    (1)			/* cache enable */
#define MMU_BUFFER_ENABLE   (1)			/* buffer enable */
#define MMU_CACHE_DISABLE   (0)			/* cache disable */
#define MMU_BUFFER_DISABLE  (0)			/* buffer disable */

#define MMU_COARSE_PAGE     (1)         /* 表示这是粗页描述符 */
#define MMU_SECTION         (2)         /* 表示这是段描述符 */
#define MMU_FINE_PAGE		(3)         /* 表示这是细页描述符 */

#define MMU_LARGE_PAGE 	    (1)         /* 表示这是大页描述符 */
#define MMU_SMALL_PAGE		(2)         /* 表示这是小页描述符 */
#define MMU_TINY_PAGE		(3)         /* 表示这是极小页描述符 */

#define MMU_COARSE_PAGE_SHIFT		(10)
#define MMU_SECTION_SHIFT			(20)
#define MMU_FINE_PAGE_SHIFT			(12)

#define MMU_LARGE_PAGE_SHIFT 	    (16)
#define MMU_SMALL_PAGE_SHIFT		(12)
#define MMU_TINY_PAGE_SHIFT			(10)

#define MMU_SECTION_SIZE	(1<<MMU_SECTION_SHIFT)
#define MMU_LARGE_PAGE_SIZE	(1<<MMU_LARGE_PAGE_SHIFT)
#define MMU_SMALL_PAGE_SIZE	(1<<MMU_SMALL_PAGE_SHIFT)
#define MMU_TINY_PAGE_SIZE 	(1<<MMU_TINY_PAGE_SHIFT)


typedef struct COARSE_PAGE {
	unsigned int type: 2;
	unsigned int blank1: 2;
	unsigned int special: 1;
	unsigned int domain: 4;
	unsigned int blank0: 1;
	unsigned int base_address: 22;
} COARSE_PAGE;

typedef struct SECTION {
	unsigned int type: 2;
	unsigned int B: 1;
	unsigned int C: 1;
	unsigned int special: 1;
	unsigned int domain: 4;
	unsigned int blank1: 1;
	unsigned int AP: 2;
	unsigned int blank0: 8;
	unsigned int base_address: 12;
} SECTION;

typedef struct FINE_PAGE {
	unsigned int type: 2;
	unsigned int blank1: 2;
	unsigned int special: 1;
	unsigned int domain: 4;
	unsigned int blank0: 3;
	unsigned int base_address: 20;
} FINE_PAGE;

typedef struct LARGE_PAGE {
	unsigned int type: 2;
	unsigned int B: 1;
	unsigned int C: 1;
	unsigned int AP0: 2;
	unsigned int AP1: 2;
	unsigned int AP2: 2;
	unsigned int AP3: 2;
	unsigned int blank0: 4;
	unsigned int base_address: 16;
} LARGE_PAGE;

typedef struct SMALL_PAGE {
	unsigned int type: 2;
	unsigned int B: 1;
	unsigned int C: 1;
	unsigned int AP0: 2;
	unsigned int AP1: 2;
	unsigned int AP2: 2;
	unsigned int AP3: 2;
	unsigned int base_address: 20;
} SMALL_PAGE;

typedef struct TINY_PAGE {
	unsigned int type: 2;
	unsigned int B: 1;
	unsigned int C: 1;
	unsigned int AP: 2;
	unsigned int blank0: 4;
	unsigned int base_address: 22;
} TINY_PAGE;

void __set_SECTION(unsigned long virtuladdr, unsigned long physicaladdr, unsigned int AP, unsigned int domain, unsigned int C, unsigned int B){
	volatile SECTION *mmu_tlb_base = (volatile SECTION *)MUM_TLB_BASE_ADDR;
	SECTION sec = {
		.base_address = physicaladdr >> MMU_SECTION_SHIFT,
		.blank0 = 0,
		.AP = AP,
		.blank1 = 0,
		.domain = domain,
		.special = 1,
		.C = C,
		.B = B,
		.type = MMU_SECTION,
	};
	mmu_tlb_base[virtuladdr >> 20] = sec;
}
void set_SECTION(unsigned long virtuladdr, unsigned long physicaladdr, unsigned long count, unsigned int AP, unsigned int domain, unsigned int C, unsigned int B){
	unsigned long vaddr = virtuladdr, paddr = physicaladdr;
	assert((vaddr&0xfffff)==0);
	assert((paddr&0xfffff)==0);
	assert(count>0);
	for (int i = 0; i < count; i++) {
		assert(vaddr>=virtuladdr);
		assert(paddr>=physicaladdr);
		__set_SECTION(vaddr, paddr, AP, domain, C, B);
		vaddr += MMU_SECTION_SIZE;
		paddr += MMU_SECTION_SIZE;
	}
}
struct mem_map{
	unsigned long virtuladdr;
	unsigned long physicaladdr;
	unsigned long size;
	unsigned int AP;
	unsigned int domain;
	unsigned int C;
	unsigned int B;
};
/*
 * 设置页表
 */
void create_page_table(void) {
	unsigned long virtuladdr, physicaladdr;
	/*
	 * Steppingstone的起始物理地址为0，第一部分程序的起始运行地址也是0，
	 * 为了在开启MMU后仍能运行第一部分的程序，
	 * 将0～1M的虚拟地址映射到同样的物理地址
	 */
	printf("正在创建Steppingstone页表项\n");
	set_SECTION(0, 0, 1, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);

	/*
	 * 0x48000000是特殊寄存器的起始物理地址，
	 * 将虚拟地址0x48000000～0x5FFFFFFF映射到物理地址0x48000000～0x5FFFFFFF上，
	 */
	printf("正在创建特殊寄存器页表项\n");
	set_SECTION(VIRTUAL_IO_ADDR, VIRTUAL_IO_ADDR, IO_MAP_SIZE >> MMU_SECTION_SHIFT, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_DISABLE, MMU_BUFFER_DISABLE);
	/*
	 * SDRAM的物理地址范围是0x30000000～0x33FFFFFF，
	 * 将虚拟地址0x30000000～0x33FFFFFF映射到物理地址0x30000000～0x33FFFFFF上，
	 * 总共64M，涉及64个段描述符
	 */
	printf("正在创建SDRAM页表项\n");
	set_SECTION(VIRTUAL_MEM_ADDR, PHYSICAL_MEM_ADDR, MEM_MAP_SIZE >> MMU_SECTION_SHIFT, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);
	//关闭framebuffer的cache
	//set_SECTION(0x33c00000, 0x33c00000, 1, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_DISABLE, MMU_BUFFER_DISABLE);
	//设置BANK4
	//TODO:不完全
	set_SECTION(0x20000000, 0x20000000, 1, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_DISABLE, MMU_BUFFER_DISABLE);
	set_SECTION(0xfff00000, 0x33f00000, 1, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);
}

/*
 * 启动MMU
 */
void mmu_init(void) {
	unsigned long ttb = (unsigned long)MUM_TLB_BASE_ADDR;;
	//创建页表
	printf("正在创建页表\n");
	create_page_table();
	printf("页表创建完成\n");
	
	__asm__ (
	    "mov    r0, #0\n"
	    "mcr    p15, 0, r0, c7, c7, 0\n"    /* 使无效ICaches和DCaches */
	    "mcr    p15, 0, r0, c7, c10, 4\n"   /* drain write buffer on v4 */
	    "mcr    p15, 0, r0, c8, c7, 0\n"    /* 使无效指令、数据TLB */

	    "mov    r4, %0\n"                   /* r4 = 页表基址 */
	    "mcr    p15, 0, r4, c2, c0, 0\n"    /* 设置页表基址寄存器 */

	    "mvn    r0, #0\n"
	    "mcr    p15, 0, r0, c3, c0, 0\n"    /* 域访问控制寄存器设为0xFFFFFFFF，
			* 不进行权限检查
			*/
	    /*
	     * 对于控制寄存器，先读出其值，在这基础上修改感兴趣的位，
	     * 然后再写入
	     */
	    "mrc    p15, 0, r0, c1, c0, 0\n"    /* 读出控制寄存器的值 */

	    /* 控制寄存器的低16位含义为：.RVI ..RS B... .CAM
	     * R : 表示换出Cache中的条目时使用的算法，
	     *     0 = Random replacement；1 = Round robin replacement
	     * V : 表示异常向量表所在的位置，
	     *     0 = Low addresses = 0x00000000；1 = High addresses = 0xFFFF0000
	     * I : 0 = 关闭ICaches；1 = 开启ICaches
	     * R、S : 用来与页表中的描述符一起确定内存的访问权限
	     * B : 0 = CPU为小字节序；1 = CPU为大字节序
	     * C : 0 = 关闭DCaches；1 = 开启DCaches
	     * A : 0 = 数据访问时不进行地址对齐检查；1 = 数据访问时进行地址对齐检查
	     * M : 0 = 关闭MMU；1 = 开启MMU
	     */

	    /*
	     * 先清除不需要的位，往下若需要则重新设置它们
	     */
	    /* .RVI ..RS B... .CAM */
	    "bic    r0, r0, #0x1000\n"          /* ...1 .... .... .... 清除I位 */
	    "bic    r0, r0, #0x0300\n"          /* .... ..11 .... .... 清除R、S位 */
	    "bic    r0, r0, #0x0087\n"          /* .... .... 1... .111 清除B/C/A/M */

	    /*
	     * 设置需要的位
	     */
		"orr    r0, r0, #0x2000\n"          /* ..1. .... .... .... 高地址中断向量表 */
	    "orr    r0, r0, #0x0002\n"          /* .... .... .... ..1. 开启对齐检查 */
	    "orr    r0, r0, #0x0004\n"          /* .... .... .... .1.. 开启DCaches */
	    "orr    r0, r0, #0x1000\n"          /* ...1 .... .... .... 开启ICaches */
	    "orr    r0, r0, #0x0001\n"          /* .... .... .... ...1 使能MMU */

	    "mcr    p15, 0, r0, c1, c0, 0\n"    /* 将修改的值写入控制寄存器 */
	    : /* 无输出 */
	    : "r" (ttb)
	    : "r0", "r4"
	);
}
void mmu_update(unsigned long ttb){
	__asm__ (
	    "mov    r0, #0\n"
	    "mcr    p15, 0, r0, c7, c7, 0\n"    /* 使无效ICaches和DCaches */
	    "mcr    p15, 0, r0, c7, c10, 4\n"   /* drain write buffer on v4 */
	    "mov    r4, %0\n"                   /* r4 = 页表基址 */
	    "mcr    p15, 0, r4, c2, c0, 0\n"    /* 设置页表基址寄存器 */
		"mcr    p15, 0, r0, c8, c7, 0\n"    /* 使无效指令、数据TLB */
	    : /* 无输出 */
	    : "r" (ttb)
	    : "r0", "r4"
	);
}
