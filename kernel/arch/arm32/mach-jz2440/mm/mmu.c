#include <global_config.h>
#include <assert.h>
#include <pgtable.h>
#include <mmu.h>
#include <memory.h>

void set_pte(pte_t *pte, U32 physicaladdr, U32 AP0, U32 AP1, U32 AP2, U32 AP3, U32 domain, U32 C, U32 B){
	SMALL_PAGE page = {
		.base_address = physicaladdr >> MMU_SMALL_PAGE_SHIFT,
		.AP0 = AP0,
		.AP1 = AP1,
		.AP2 = AP2,
		.AP3 = AP3,
		.C = C,
		.B = B,
		.type = MMU_SMALL_PAGE,
	};
	pte[0] = *(pte_t *)&page;
}

pte_t *alloc_pte(pmd_t *pmd){
	U32 temp = pmd[0];
	U32 *paddr;
	SECTION sec = *(SECTION *)pmd;
	if(temp&0x3 == MMU_COARSE_PAGE)
		assert(0);

	U32 *p = kmalloc(PAGE_2K_SIZE, 0);
	assert(p);
	pte_t *pte = p;

	COARSE_PAGE page = {
		.type = MMU_COARSE_PAGE,
		.blank1 = 0,
		.special = 1,
		.blank0 = 0,
		.base_address = Virt_To_Phy(p) >> MMU_COARSE_PAGE_SHIFT,
	};

	if(temp&0x3 == MMU_SECTION) {
		page.domain = sec.domain;
		SMALL_PAGE small_page = {
			.base_address = (sec.base_address << MMU_SECTION_SHIFT)>> MMU_SMALL_PAGE_SHIFT,
			.AP0 = sec.AP,
			.AP1 = sec.AP,
			.AP2 = sec.AP,
			.AP3 = sec.AP,
			.C = sec.C,
			.B = sec.B,
			.type = MMU_SMALL_PAGE,
		};
		
		for(int i=0;i<512;i++){
			pte[i] = *(pte_t *)&small_page;
			small_page.base_address++;
		}
	}else{
		//TODO
		page.domain = MMU_DOMAIN(0);
		memset(pte, 0, 512*4);
	}
	
	pmd[0] = *(pmd_t *)&page;
	page.base_address++;
	pmd[1] = *(pmd_t *)&page;

	return pte;
}

void set_pmd(pmd_t *pmd, U32 physicaladdr, U32 AP, U32 domain, U32 C, U32 B){
	U32 temp = pmd[0];
	if(temp&0x3 == MMU_COARSE_PAGE)
		assert(0);
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
	pmd[0] = *(pmd_t *)&sec;
	sec.base_address++;
	pmd[1] = *(pmd_t *)&sec;
}

void set_pgd(pgd_t *pgd, U32 physicaladdr, U32 AP, U32 domain, U32 C, U32 B){
	assert(!(physicaladdr&(PGDIR_SIZE-1)));
	pmd_t *pmd = pgd;
	set_pmd(pmd, physicaladdr, AP, domain, C, B);
}

static void alloc_init_pgd(pgd_t *pgd, U32 virtuladdr, U32 physicaladdr, U32 count, U32 AP, U32 domain, U32 C, U32 B){
	U32 vaddr = virtuladdr, paddr = physicaladdr;
	assert(!(vaddr& (PGDIR_SIZE-1)));
	assert(!(paddr& (PGDIR_SIZE-1)));
	assert(count>0);
	for (int i = 0; i < count; i++) {
		assert(vaddr>=virtuladdr);
		assert(paddr>=physicaladdr);
		pmd_t *pmd = pgd + pgd_index(vaddr);
		set_pmd(pmd, paddr, AP, domain, C, B);
		vaddr += PGDIR_SIZE;
		paddr += PGDIR_SIZE;
	}
}

/*
 * 设置页表
 */
void create_page_table(void) {
	pgd_t *pgd = (pgd_t *)MUM_TLB_BASE_ADDR;
	memset(pgd, 0, 16*1024);
	/*
	 * Steppingstone的起始物理地址为0，第一部分程序的起始运行地址也是0，
	 * 为了在开启MMU后仍能运行第一部分的程序，
	 * 将0～1M的虚拟地址映射到同样的物理地址
	 */
	printf("正在创建Steppingstone页表项\n");
	alloc_init_pgd(pgd, 0, 0, 1, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);

	/*
	 * 0x48000000是特殊寄存器的起始物理地址，
	 * 将虚拟地址0x48000000～0x5FFFFFFF映射到物理地址0x48000000～0x5FFFFFFF上，
	 */
	printf("正在创建特殊寄存器页表项\n");
	alloc_init_pgd(pgd, VIRTUAL_IO_ADDR, VIRTUAL_IO_ADDR, IO_MAP_SIZE >> PGDIR_SHIFT, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_DISABLE, MMU_BUFFER_DISABLE);
	/*
	 * SDRAM的物理地址范围是0x30000000～0x33FFFFFF，
	 * 将虚拟地址0x30000000～0x33FFFFFF映射到物理地址0x30000000～0x33FFFFFF上，
	 * 总共64M，涉及64个段描述符
	 */
	printf("正在创建SDRAM页表项\n");
	alloc_init_pgd(pgd, VIRTUAL_MEM_ADDR, PHYSICAL_MEM_ADDR, MEM_MAP_SIZE >> PGDIR_SHIFT, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);
	//关闭framebuffer的cache
	//set_SECTION(0x33c00000, 0x33c00000, 1, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_DISABLE, MMU_BUFFER_DISABLE);
	//设置BANK4
	//TODO:不完全
	alloc_init_pgd(pgd, 0x20000000, 0x20000000, 1, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_DISABLE, MMU_BUFFER_DISABLE);
}

extern void arm920_flush_kern_cache_all();

void set_vector_map(){
	pgd_t *pgd = (pgd_t *)MUM_TLB_BASE_ADDR;

	memcpy(0x33fff000, 0x30100000, 4096);
	pmd_t *pmd = pgd + pgd_index(0xffff0000);
	pte_t *pte = alloc_pte(pmd);
	pte = pte + pte_index(0xffff0000);
	set_pte(pte, 0x33fff000, MMU_FULL_ACCESS, MMU_FULL_ACCESS, MMU_FULL_ACCESS, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);
	flush_cache();
	flush_tlb();
	assert(*(int *)0x33fff000 == *(int *)0xffff0000);
	//mmu_test();
}
/*
 * 启动MMU
 */
void mmu_init(void) {
	unsigned long ttb = (unsigned long)MUM_TLB_BASE_ADDR;
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

void mmu_test(){
	int *p;
	pgd_t *pgd;

	p = 0x32000000;
	p[0] = 0x12345678;
	
	p = 0xe2000000;
	pgd = (pgd_t *)MUM_TLB_BASE_ADDR + pgd_index(0xe2000000);
	//set_pgd(pgd, 0x32400000, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);
	//flush_pmd_entry(pgd);
	//printf("[0xe2000000] = %#X\n", *(int *)0xe2000000);
	set_pgd(pgd, 0x32000000, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);
	flush_pmd_entry(pgd);
	printf("[0xe2000000] = %#X\n", *(int *)0xe2000000);


	p = 0x32200000;
	printf("[0x32200000] = %#X\n", *(int *)0x32200000);
	pgd = (pgd_t *)MUM_TLB_BASE_ADDR + pgd_index(0x32000000);
	set_pgd(pgd, p, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);
	
	flush_tlb();
	flush_cache();
	printf("[0x32000000] = %#X\n", *(int *)0x32000000);

	while(1);
}
