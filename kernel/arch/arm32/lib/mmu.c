/*
 * mmu.c
 */

#include <stdio.h>
#include <assert.h>
#include <pgtable.h>
#include <memory.h>
#include <arm32.h>
#include <mmu.h>
#include <ldscript.h>

/*
 * MMU translation table base address
 */
static u32_t __mmu_ttb[4096] __attribute__((aligned(0x4000)));

#define MUM_TLB_BASE_ADDR  (__mmu_ttb)

static void map_l1_section(virtual_addr_t virt, physical_addr_t phys, physical_size_t size, int type)
{
	physical_size_t i;

	virt >>= 20;
	phys >>= 20;
	size >>= 20;
	type &= 0x3;

	for(i = size; i > 0; i--, virt++, phys++)
		__mmu_ttb[virt] = (phys << 20) | (0x3 << 10) | (0x0 << 5) | (type << 2) | (0x2 << 0);
}

void set_pte(pte_t *pte, u32_t physicaladdr, u32_t AP, u32_t map_type){
	SMALL_PAGE page = {
		.base_address = physicaladdr >> MMU_SMALL_PAGE_SHIFT,
		.AP = AP,
		.AP2 = 0,
		.CB = map_type,
		.XN = 0,
		.one = 1,
	};
	((SMALL_PAGE *)pte)[0] = page;
}

pte_t *alloc_pte(pmd_t *pmd){
	u32_t temp = pmd[0];
	SECTION sec = *(SECTION *)pmd;
	if((temp & 0x3) == MMU_PAGE_TABLE)
		assert(0);

	u32_t *p = kmalloc(PAGE_2K_SIZE, 0);
	assert(p);
	pte_t *pte = p;

	PAGE_TABLE page = {
		.type = MMU_PAGE_TABLE,
		.domain = 0,
		.NS = 1,
		.base_address = virt_to_phy(p) >> MMU_PAGE_TABLE_SHIFT,
	};

	if((temp & 0x3) == MMU_SECTION) {
		for(int i = 0; i < 512; i++){
			u32_t addr = (sec.base_address << MMU_SECTION_SHIFT) + (i << MMU_SMALL_PAGE_SHIFT);
			set_pte(pte + i, addr, sec.AP, sec.CB);
		}
	}else{
		memset(pte, 0, PAGE_2K_SIZE);
	}

	((PAGE_TABLE *)pmd)[0] = page;
	page.base_address++;
	((PAGE_TABLE *)pmd)[1] = page;

	return pte;
}

void set_pmd(pmd_t *pmd, u32_t physicaladdr, u32_t AP, u32_t map_type){
	u32_t temp = pmd[0];
	if((temp & 0x3) == MMU_PAGE_TABLE)
		assert(0);
	SECTION sec = {
		.base_address = physicaladdr >> MMU_SECTION_SHIFT,
		.AP = AP,
		.domain = MMU_DOMAIN(0),
		.CB = map_type,
		.type = MMU_SECTION,
	};
	((SECTION *)pmd)[0] = sec;
	sec.base_address++;
	((SECTION *)pmd)[1] = sec;
}

void set_pgd(pgd_t *pgd, u32_t physicaladdr, u32_t AP, u32_t map_type){
	assert(!(physicaladdr&(PGDIR_SIZE-1)));
	pmd_t *pmd = (pmd_t *)pgd;
	set_pmd(pmd, physicaladdr, AP, map_type);
}

static void alloc_init_pgd(pgd_t *pgd, u32_t virtuladdr, u32_t physicaladdr, u32_t count, u32_t AP, u32_t map_type){
	u32_t vaddr = virtuladdr, paddr = physicaladdr;
	assert(!(vaddr& (PGDIR_SIZE-1)));
	assert(!(paddr& (PGDIR_SIZE-1)));
	assert(count>0);
	for (int i = 0; i < count; i++) {
		assert(vaddr>=virtuladdr);
		assert(paddr>=physicaladdr);
		pmd_t *pmd = (pmd_t *)(pgd + pgd_index(vaddr));
		set_pmd(pmd, paddr, AP, map_type);
		vaddr += PGDIR_SIZE;
		paddr += PGDIR_SIZE;
	}
}

void set_vector_map(pgd_t *pgd){
	pmd_t *pmd = (pmd_t *)(pgd + pgd_index(0xffff0000));
	pte_t *pte = alloc_pte(pmd);
	pte = pte + pte_index(0xffff0000);
	set_pte(pte, &__text_start, MMU_FULL_ACCESS, MAP_TYPE_CB);
	LOG("[exception vector map]");
	LOG("<%08X %08X> ==> <%08X %08X>\n", 0xffff0000, 0xffff0000 + 0x1000, &__text_start, &__text_start + 0x1000);
	//flush_cache();
	//flush_tlb();
	//mmu_test();
}

void create_page_table(struct machine_t * mach, pgd_t *pgd) {
	memset(pgd, 0, 16 * 1024);

	struct mmap_t * pos, * n;

	LOG("[memory map]");
	if(mach)
	{
		list_for_each_entry_safe(pos, n, &mach->mmap, list)
		{
			alloc_init_pgd(pgd, pos->virt, pos->phys, pos->size >> PGDIR_SHIFT, MMU_FULL_ACCESS, pos->type);
			LOG("<%08X %08X> ==> <%08X %08X>", pos->virt, pos->virt + pos->size, pos->phys, pos->phys + pos->size);
		}
	}
	set_vector_map(pgd);
}

void mmu_setup(struct machine_t * mach) {

	pgd_t *pgd = (pgd_t *)MUM_TLB_BASE_ADDR;

	create_page_table(mach, pgd);

	arm32_ttb_set((u32_t)(pgd));
	arm32_tlb_invalidate();
	arm32_domain_set(0x3);
	arm32_set_high_exception_vectors();
	arm32_mmu_enable();
	arm32_icache_enable();
	arm32_dcache_enable();
}
void * mmu_get_tlb_base_addr() {
	return MUM_TLB_BASE_ADDR;
}
void mmu_test(){
	int *p;
	pgd_t *pgd;

	p = (int *)0x32000000;
	p[0] = 0x12345678;
	
	p = (int *)0xe2000000;
	pgd = (pgd_t *)MUM_TLB_BASE_ADDR + pgd_index(0xe2000000);
	//set_pgd(pgd, 0x32400000, MMU_FULL_ACCESS, MAP_TYPE_CB);
	//flush_pmd_entry(pgd);
	//LOG("[0xe2000000] = %#p", *(int *)0xe2000000);
	set_pgd(pgd, 0x32000000, MMU_FULL_ACCESS, MAP_TYPE_CB);
	flush_pmd_entry(pgd);
	LOG("[0xe2000000] = %#p", *(int *)0xe2000000);


	p = (int *)0x32200000;
	LOG("[0x32200000] = %#p", *(int *)0x32200000);
	pgd = (pgd_t *)MUM_TLB_BASE_ADDR + pgd_index(0x32000000);
	set_pgd(pgd, (u32_t)p, MMU_FULL_ACCESS, MAP_TYPE_CB);
	
	flush_tlb();
	flush_cache();
	LOG("[0x32000000] = %#p", *(int *)0x32000000);

	while(1);
}
