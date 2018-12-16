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
#include "ldscript.h"
#include "assert.h"
#include "task.h"
#include "memory.h"
#include "printk.h"
#include "lib.h"
#include "errno.h"
#include "mmu.h"

struct Global_Memory_Descriptor mms = {{0}, 0};

//// each zone index

int ZONE_DMA_INDEX	= 0;
int ZONE_NORMAL_INDEX	= 0;	//low 4GB RAM ,was mapped in pagetable
int ZONE_UNMAPED_INDEX	= 0;	//above 4GB RAM,unmapped in pagetable

struct Slab_cache kmalloc_cache_size[16] = {
	{32		, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{64		, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{128	, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{256	, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{512	, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{1024	, 0	, 0	, NULL	, NULL	, NULL	, NULL},			//1KB
	{2048	, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{4096	, 0	, 0	, NULL	, NULL	, NULL	, NULL},			//4KB
	{8192	, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{16384	, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{32768	, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{65536	, 0	, 0	, NULL	, NULL	, NULL	, NULL},			//64KB
	{131072	, 0	, 0	, NULL	, NULL	, NULL	, NULL},			//128KB
	{262144	, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{524288	, 0	, 0	, NULL	, NULL	, NULL	, NULL},
	{1048576, 0	, 0	, NULL	, NULL	, NULL	, NULL},			//1MB
};


unsigned long page_init(struct Page *page, unsigned long flags) {
	page->attribute |= flags;

	if (!page->reference_count || (page->attribute & PG_Shared)) {
		page->reference_count++;
		page->zone_struct->total_pages_link++;
	}

	return 1;
}


unsigned long page_clean(struct Page *page) {
	page->reference_count--;
	page->zone_struct->total_pages_link--;

	if (!page->reference_count) {
		page->attribute &= PG_PTable_Maped;
	}

	return 1;
}

unsigned long get_page_attribute(struct Page *page) {
	if (page == NULL) {
		color_printk(RED, BLACK, "get_page_attribute() ERROR: page == NULL\n");
		return 0;
	} else
		return page->attribute;
}

unsigned long set_page_attribute(struct Page *page, unsigned long flags) {
	if (page == NULL) {
		color_printk(RED, BLACK, "set_page_attribute() ERROR: page == NULL\n");
		return 0;
	} else {
		page->attribute = flags;
		return 1;
	}
}

void init_memory() {
	int i, j;
	unsigned long TotalMem = 0 ;
	struct E820 *p = NULL;
	unsigned long *tmp = NULL;

	color_printk(BLUE, BLACK, "Display Physics Address MAP,Type(1:RAM,2:ROM or Reserved,3:ACPI Reclaim Memory,4:ACPI NVS Memory,Others:Undefine)\n");
	p = (struct E820[2]) {
		[0] = {
			.address = 0x30000000,
			.length = 0x04000000,
			.type = 1,
		},
		[1] = {
			.address = 0,
			.length = 0,
			.type = 0,
		},
	};

	mms.start_code = (unsigned long)& _text;
	mms.end_code   = (unsigned long)& _etext;
	mms.end_data   = (unsigned long)& _edata;
	mms.end_rodata = (unsigned long)& _erodata;
	mms.start_brk  = (unsigned long)& _end;

	
	for (i = 0; i < 32; i++) {
		if (p->type == 1)
			color_printk(ORANGE, BLACK, "Address:%#010lx\tLength:%#010lx\tType:%#010x\n", p->address, p->length, p->type);
		if (p->type == 1)
			TotalMem +=  p->length;

		mms.e820[i].address = p->address;
		mms.e820[i].length	= p->length;
		mms.e820[i].type	= p->type;
		mms.e820_length = i;
		p++;
		if (p->type > 4 || p->length == 0 || p->type < 1)
			break;
	}

	color_printk(ORANGE, BLACK, "OS Can Used Total RAM:%#010lx\n", TotalMem);

	TotalMem = 0;

	for (i = 0; i <= mms.e820_length; i++) {
		unsigned long start, end;
		if (mms.e820[i].type != 1)
			continue;
		start = PAGE_2M_ALIGN(mms.e820[i].address);
		end   = (mms.e820[i].address + mms.e820[i].length) & PAGE_2M_MASK;
		if (end <= start)
			continue;
		TotalMem += PAGE_2M_NUM(end - start);
	}

	color_printk(ORANGE, BLACK, "OS Can Used Total 2M PAGEs:%#010lx=%d\n", TotalMem, TotalMem);

	TotalMem = mms.e820[mms.e820_length].address + mms.e820[mms.e820_length].length;

	//bits map construction init
	mms.bits_map = (unsigned long *)ALIGN((unsigned long)mms.start_brk, PAGE_4K_SIZE);
	mms.bits_size = PAGE_2M_NUM(TotalMem);
	mms.bits_length = ALIGN(ALIGN(TotalMem >> PAGE_2M_SHIFT, 8) / 8, sizeof(long));
	memset(mms.bits_map, 0xff, mms.bits_length);		//init bits map memory

	//pages construction init
	mms.pages_struct = (struct Page *)ALIGN((unsigned long)mms.bits_map + mms.bits_length, PAGE_4K_SIZE - 1);
	mms.pages_size = PAGE_2M_NUM(TotalMem);
	mms.pages_length = ALIGN((TotalMem >> PAGE_2M_SHIFT) * sizeof(struct Page), sizeof(long));
	memset(mms.pages_struct, 0x00, mms.pages_length);	//init pages memory

	//zones construction init
	mms.zones_struct = (struct Zone *)ALIGN((unsigned long)mms.pages_struct + mms.pages_length, PAGE_4K_SIZE);
	mms.zones_size   = 0;
	mms.zones_length = ALIGN(5 * sizeof(struct Zone), sizeof(long));
	memset(mms.zones_struct, 0x00, mms.zones_length);	//init zones memory

	for (i = 0; i <= mms.e820_length; i++) {
		unsigned long start, end;
		struct Zone *z;
		struct Page *p;

		if (mms.e820[i].type != 1)
			continue;
		start = PAGE_2M_ALIGN(mms.e820[i].address);
		end   = (mms.e820[i].address + mms.e820[i].length) & PAGE_2M_MASK;
		if (end <= start)
			continue;

		//zone init

		z = mms.zones_struct + mms.zones_size;
		mms.zones_size++;

		z->zone_start_address = start;
		z->zone_end_address = end;
		z->zone_length = end - start;

		z->page_using_count = 0;
		z->page_free_count = PAGE_2M_NUM(end - start);

		z->total_pages_link = 0;

		z->attribute = 0;
		z->GMD_struct = &mms;

		z->pages_length = PAGE_2M_NUM(end - start);
		z->pages_group = mms.pages_struct + PAGE_2M_NUM(start);

		//page init
		p = z->pages_group;
		for (j = 0; j < z->pages_length; j++ , p++) {
			p->zone_struct = z;
			p->PHY_address = start + PAGE_2M_SIZE * j;
			p->attribute = 0;
			p->reference_count = 0;
			p->age = 0;

			BIT_NOT(mms.bits_map, PAGE_2M_NUM(p->PHY_address));
		}
	}

	/////////////init address 0 to page struct 0; because the memory_management_struct.e820[0].type != 1

	mms.pages_struct->zone_struct = mms.zones_struct;
	mms.pages_struct->PHY_address = 0UL;
	set_page_attribute(mms.pages_struct, PG_PTable_Maped | PG_Kernel_Init | PG_Kernel);
	mms.pages_struct->reference_count = 1;
	mms.pages_struct->age = 0;

	/////////////

	mms.zones_length = ALIGN(mms.zones_size * sizeof(struct Zone), sizeof(long));

	color_printk(ORANGE, BLACK, "bits_map:%#010lx,bits_size:%#010lx,bits_length:%#010lx\n", mms.bits_map, mms.bits_size, mms.bits_length);
	color_printk(ORANGE, BLACK, "pages_struct:%#010lx,pages_size:%#010lx,pages_length:%#010lx\n", mms.pages_struct, mms.pages_size, mms.pages_length);
	color_printk(ORANGE, BLACK, "zones_struct:%#010lx,zones_size:%#010lx,zones_length:%#010lx\n", mms.zones_struct, mms.zones_size, mms.zones_length);

	ZONE_DMA_INDEX = 0;
	ZONE_NORMAL_INDEX = 0;
	ZONE_UNMAPED_INDEX = 0;

	for (i = 0; i < mms.zones_size; i++) {
		struct Zone *z = mms.zones_struct + i;
		color_printk(ORANGE, BLACK, "zone_start_address:%#010lx,zone_end_address:%#010lx,zone_length:%#010lx,pages_group:%#010lx,pages_length:%#010lx\n", z->zone_start_address, z->zone_end_address, z->zone_length, z->pages_group, z->pages_length);

		//if (z->zone_start_address >= 0x100000000 && !ZONE_UNMAPED_INDEX)
		//	ZONE_UNMAPED_INDEX = i;
	}

	color_printk(ORANGE, BLACK, "ZONE_DMA_INDEX:%d\tZONE_NORMAL_INDEX:%d\tZONE_UNMAPED_INDEX:%d\n", ZONE_DMA_INDEX, ZONE_NORMAL_INDEX, ZONE_UNMAPED_INDEX);

	mms.end_of_struct = ALIGN((unsigned long)mms.zones_struct + mms.zones_length + sizeof(long) * 32, sizeof(long));	////need a blank to separate memory_management_struct

	color_printk(ORANGE, BLACK, "start_code:%#010lx,end_code:%#010lx,end_data:%#010lx,start_brk:%#010lx,end_of_struct:%#010lx\n", mms.start_code, mms.end_code, mms.end_data, mms.start_brk, mms.end_of_struct);

	i = PAGE_2M_NUM(Virt_To_Phy(mms.end_of_struct));

	for (j = 1; j <= i; j++) {
		struct Page *tmp_page = mms.pages_struct + j;
		page_init(tmp_page, PG_PTable_Maped | PG_Kernel_Init | PG_Kernel);
		BIT_SET(mms.bits_map, PAGE_2M_NUM(tmp_page->PHY_address));
		tmp_page->zone_struct->page_using_count++;
		tmp_page->zone_struct->page_free_count--;
	}

	//tmp = Get_gdt();

	//color_printk(INDIGO, BLACK, "tmp\t:%#010lx\n", tmp);
	//color_printk(INDIGO, BLACK, "*tmp\t:%#010lx\n", *Phy_To_Virt(tmp) & (~0xff));
	//color_printk(PURPLE, BLACK, "**tmp\t:%#010lx\n", *Phy_To_Virt(*Phy_To_Virt(tmp) & (~0xff)) & (~0xff));
	color_printk(ORANGE, BLACK, "1.mms.bits_map:%#010lx\tzone_struct->page_using_count:%d\tzone_struct->page_free_count:%d\n", *mms.bits_map, mms.zones_struct->page_using_count, mms.zones_struct->page_free_count);

//	for(i = 0;i < 10;i++)
//		*(Phy_To_Virt(Global_CR3)  + i) = 0UL;

	flush_tlb();
}

/*

	number: number < LONG_BIT_NUM
	zone_select: zone select from dma , mapped in  pagetable , unmapped in pagetable
	page_flags: struct Page flages

*/

struct Page *alloc_pages(int zone_select, int number, unsigned long page_flags) {
	unsigned long page = 0;
	unsigned long attribute = 0;

	int zone_start = 0;
	int zone_end = 0;

	if (number >= BITS_PER_LONG || number <= 0) {
		color_printk(RED, BLACK, "alloc_pages() ERROR: number is invalid\n");
		goto ret_null;
	}

	switch (zone_select) {
		case ZONE_DMA:
			zone_start = 0;
			zone_end = ZONE_DMA_INDEX;
			attribute = PG_PTable_Maped;
			break;

		case ZONE_NORMAL:
			zone_start = ZONE_DMA_INDEX;
			zone_end = ZONE_NORMAL_INDEX;
			attribute = PG_PTable_Maped;
			break;

		case ZONE_UNMAPED:
			zone_start = ZONE_UNMAPED_INDEX;
			zone_end = mms.zones_size - 1;
			attribute = 0;
			break;

		default:
			color_printk(RED, BLACK, "alloc_pages() ERROR: zone_select index is invalid\n");
			goto ret_null;
			break;
	}

	for (int i = zone_start; i <= zone_end; i++) {
		struct Zone *z;
		unsigned long start, end;
		unsigned long tmp;

		if (mms.zones_struct[i].page_free_count < number)
			continue;

		z = mms.zones_struct + i;
		start = PAGE_2M_NUM(z->zone_start_address);
		end = (z->zone_end_address);

		for (unsigned long j = start; j < end; j++) {
			unsigned long *p = mms.bits_map + (j / BITS_PER_LONG);
			unsigned long num = (1UL << number) - 1;
			int k = j % BITS_PER_LONG;
			if (!((p[0] >> k | p[!!k] << (BITS_PER_LONG - k)) & num)) {
				struct Page *pageptr = mms.pages_struct + j;
				for (int i = 0; i < number; i++) {
					BIT_SET(mms.bits_map, PAGE_2M_NUM(pageptr[i].PHY_address));
					pageptr[i].attribute = attribute;
				}
				z->page_using_count += number;
				z->page_free_count -= number;
				page = j;
				goto find_free_pages;
			}
		}
	}

	color_printk(RED, BLACK, "alloc_pages() ERROR: no page can alloc\n");
ret_null:
	assert(0);
	return NULL;

find_free_pages:

	return &(mms.pages_struct[page]);
}

/*

	page: free page start from this pointer
	number: number < LONG_BIT_NUM

*/

void free_pages(struct Page *page, int number) {
	if (page == NULL) {
		color_printk(RED, BLACK, "free_pages() ERROR: page is invalid\n");
		return ;
	}

	if (number >= BITS_PER_LONG || number <= 0) {
		color_printk(RED, BLACK, "free_pages() ERROR: number is invalid\n");
		return ;
	}

	for (int i = 0; i < number; i++, page++) {
		BIT_CLR(mms.bits_map, PAGE_2M_NUM(page->PHY_address));
		page->zone_struct->page_using_count--;
		page->zone_struct->page_free_count++;
		page->attribute = 0;
	}
}

/*
 return virtual kernel address
 gfp_flages: the condition of get memory
*/

void *kmalloc(unsigned long size, unsigned long gfp_flages) {
	int i;
	struct Slab *slab = NULL;
	if (size > 1048576) {
		color_printk(RED, BLACK, "kmalloc() ERROR: kmalloc size too long:%08d\n", size);
		goto ret_null;
	}
	for (i = 0; i < ARRAY_SIZE(kmalloc_cache_size); i++)
		if (kmalloc_cache_size[i].size >= size)
			break;
	slab = kmalloc_cache_size[i].cache_pool;

	if (kmalloc_cache_size[i].total_free != 0) {
		while (1) {
			if (slab->free_count == 0)
				slab = container_of(list_next(&slab->list), struct Slab, list);
			else
				break;
			if (slab == kmalloc_cache_size[i].cache_pool) {
				color_printk(BLUE, BLACK, "kmalloc() ERROR: kmalloc_cache_size[i].total_free != 0\n");
				goto ret_null;
			}
		}
	} else {
		slab = kmalloc_create(kmalloc_cache_size[i].size);

		if (slab == NULL) {
			color_printk(BLUE, BLACK, "kmalloc()->kmalloc_create()=>slab == NULL\n");
			goto ret_null;
		}

		kmalloc_cache_size[i].total_free += slab->color_count;

		color_printk(BLUE, BLACK, "kmalloc()->kmalloc_create()<=size:%#010x\n", kmalloc_cache_size[i].size); ///////

		list_add_to_before(&kmalloc_cache_size[i].cache_pool->list, &slab->list);
	}

	for (int j = 0; j < slab->color_count; j++) {
		if (slab->color_map[j / BITS_PER_LONG] == 0xffffffffffffffffUL) {
			j += BITS_PER_LONG - 1;
			continue;
		}

		if ((slab->color_map[j / BITS_PER_LONG] & (1UL << (j % BITS_PER_LONG))) == 0) {
			BIT_SET(slab->color_map, j);
			slab->using_count++;
			slab->free_count--;

			kmalloc_cache_size[i].total_free--;
			kmalloc_cache_size[i].total_using++;

			return (void *)((char *)slab->Vaddress + kmalloc_cache_size[i].size * j);
		}
	}

	color_printk(BLUE, BLACK, "kmalloc() ERROR: no memory can alloc\n");
ret_null:
	assert(0);
	return NULL;
}

/*

*/

struct Slab *kmalloc_create(unsigned long size) {
	struct Slab *slab = NULL;
	struct Page *page = NULL;
	unsigned long *vaddresss = NULL;
	long structsize = 0;

	page = alloc_pages(ZONE_NORMAL, 1, 0);

	if (page == NULL) {
		color_printk(RED, BLACK, "kmalloc_create()->alloc_pages()=>page == NULL\n");
		return NULL;
	}

	page_init(page, PG_Kernel);

	switch (size) {
			////////////////////slab + map in 2M page

		case 32:
		case 64:
		case 128:
		case 256:
		case 512:

			vaddresss = Phy_To_Virt(page->PHY_address);
			structsize = sizeof(struct Slab) + PAGE_2M_SIZE / size / 8;

			slab = (struct Slab *)((unsigned char *)vaddresss + PAGE_2M_SIZE - structsize);
			slab->free_count = (PAGE_2M_SIZE - structsize) / size;
			slab->using_count = 0;
			slab->color_count = slab->free_count;
			slab->color_length = ALIGN(ALIGN(slab->color_count, 8) / 8, sizeof(unsigned long));
			slab->color_map = (unsigned long *)((unsigned char *)slab + sizeof(struct Slab));

			break;

			///////////////////kmalloc slab and map,not in 2M page anymore

		case 1024:		//1KB
		case 2048:
		case 4096:		//4KB
		case 8192:
		case 16384:

			//////////////////color_map is a very short buffer.

		case 32768:
		case 65536:
		case 131072:		//128KB
		case 262144:
		case 524288:
		case 1048576:		//1MB

			slab = (struct Slab *)kmalloc(sizeof(struct Slab), 0);
			slab->free_count = PAGE_2M_SIZE / size;
			slab->using_count = 0;
			slab->color_count = slab->free_count;
			slab->color_length = ALIGN(ALIGN(slab->color_count, 8) / 8, sizeof(unsigned long));
			slab->color_map = (unsigned long *)kmalloc(slab->color_length, 0);

			break;

		default:

			color_printk(RED, BLACK, "kmalloc_create() ERROR: wrong size:%08d\n", size);
			free_pages(page, 1);

			return NULL;
	}

	slab->Vaddress = Phy_To_Virt(page->PHY_address);
	slab->page = page;
	memset(slab->color_map, 0xff, slab->color_length);
	for (int i = 0; i < slab->color_count; i++)
		BIT_NOT(slab->color_map, i);
	list_init(&slab->list);

	return slab;
}

/*

*/

unsigned long kfree(void *address) {
	struct Slab *slab = NULL;
	void *page_base_address = (void *)((unsigned long)address & PAGE_2M_MASK);

	for (int i = 0; i < ARRAY_SIZE(kmalloc_cache_size); i++) {
		slab = kmalloc_cache_size[i].cache_pool;
		do {
			if (slab->Vaddress == page_base_address) {
				int index = (address - slab->Vaddress) / kmalloc_cache_size[i].size;
				BIT_NOT(slab->color_map, index);

				slab->free_count++;
				slab->using_count--;

				kmalloc_cache_size[i].total_free++;
				kmalloc_cache_size[i].total_using--;

				if ((slab->using_count == 0) && (kmalloc_cache_size[i].total_free >= slab->color_count * 3 / 2) && (kmalloc_cache_size[i].cache_pool != slab)) {
					switch (kmalloc_cache_size[i].size) {
						////////////////////slab + map in 2M page

						case 32:
						case 64:
						case 128:
						case 256:
						case 512:
							list_del(&slab->list);
							kmalloc_cache_size[i].total_free -= slab->color_count;

							page_clean(slab->page);
							free_pages(slab->page, 1);
							break;

						default:
							list_del(&slab->list);
							kmalloc_cache_size[i].total_free -= slab->color_count;

							kfree(slab->color_map);

							page_clean(slab->page);
							free_pages(slab->page, 1);
							kfree(slab);
							break;
					}

				}
				return 1;
			} else
				slab = container_of(list_next(&slab->list), struct Slab, list);
		} while(slab != kmalloc_cache_size[i].cache_pool);
	}

	color_printk(RED, BLACK, "kfree() ERROR: can`t free memory\n");
	return 0;
}
/*

*/

struct Slab_cache *slab_create(unsigned long size, void * (* constructor)(void *Vaddress, unsigned long arg), void * (* destructor)(void *Vaddress, unsigned long arg), unsigned long arg) {
	struct Slab_cache *slab_cache = NULL;

	slab_cache = (struct Slab_cache *)kmalloc(sizeof(struct Slab_cache), 0);

	if (slab_cache == NULL) {
		color_printk(RED, BLACK, "slab_create()->kmalloc()=>slab_cache == NULL\n");
		return NULL;
	}

	memset(slab_cache, 0, sizeof(struct Slab_cache));

	slab_cache->size = SIZEOF_LONG_ALIGN(size);
	slab_cache->total_using = 0;
	slab_cache->total_free = 0;
	slab_cache->cache_pool = (struct Slab *)kmalloc(sizeof(struct Slab), 0);

	if (slab_cache->cache_pool == NULL) {
		color_printk(RED, BLACK, "slab_create()->kmalloc()=>slab_cache->cache_pool == NULL\n");
		kfree(slab_cache);
		return NULL;
	}

	memset(slab_cache->cache_pool, 0, sizeof(struct Slab));

	slab_cache->cache_dma_pool = NULL;
	slab_cache->constructor = constructor;
	slab_cache->destructor = destructor;

	list_init(&slab_cache->cache_pool->list);

	slab_cache->cache_pool->page = alloc_pages(ZONE_NORMAL, 1, 0);

	if (slab_cache->cache_pool->page == NULL) {
		color_printk(RED, BLACK, "slab_create()->alloc_pages()=>slab_cache->cache_pool->page == NULL\n");
		kfree(slab_cache->cache_pool);
		kfree(slab_cache);
		return NULL;
	}

	page_init(slab_cache->cache_pool->page, PG_Kernel);

	slab_cache->cache_pool->using_count = 0;
	slab_cache->cache_pool->free_count = PAGE_2M_SIZE / slab_cache->size;
	slab_cache->total_free = slab_cache->cache_pool->free_count;
	slab_cache->cache_pool->Vaddress = Phy_To_Virt(slab_cache->cache_pool->page->PHY_address);
	slab_cache->cache_pool->color_count = slab_cache->cache_pool->free_count;
	slab_cache->cache_pool->color_length = (ALIGN(slab_cache->cache_pool->color_count, 8) / 8, sizeof(unsigned long));
	slab_cache->cache_pool->color_map = (unsigned long *)kmalloc(slab_cache->cache_pool->color_length, 0);

	if (slab_cache->cache_pool->color_map == NULL) {
		color_printk(RED, BLACK, "slab_create()->kmalloc()=>slab_cache->cache_pool->color_map == NULL\n");

		free_pages(slab_cache->cache_pool->page, 1);
		kfree(slab_cache->cache_pool);
		kfree(slab_cache);
		return NULL;
	}

	memset(slab_cache->cache_pool->color_map, 0, slab_cache->cache_pool->color_length);

	return slab_cache;
}

/*

*/

unsigned long slab_destroy(struct Slab_cache *slab_cache) {
	struct Slab *slab = slab_cache->cache_pool;

	if (slab_cache->total_using != 0) {
		color_printk(RED, BLACK, "slab_cache->total_using != 0\n");
		return 0;
	}

	while (!list_is_empty(&slab->list)) {
		list_del(&slab->list);
		kfree(slab->color_map);

		page_clean(slab->page);
		free_pages(slab->page, 1);
		kfree(slab);
		slab = container_of(list_next(&slab->list), struct Slab, list);
	}

	kfree(slab->color_map);

	page_clean(slab->page);
	free_pages(slab->page, 1);
	kfree(slab);

	kfree(slab_cache);

	return 1;
}

/*

*/

void *slab_malloc(struct Slab_cache *slab_cache, unsigned long arg) {
	struct Slab *slab = slab_cache->cache_pool;

	if (slab_cache->total_free == 0) {
		slab = (struct Slab *)kmalloc(sizeof(struct Slab), 0);

		if (slab == NULL) {
			color_printk(RED, BLACK, "slab_malloc()->kmalloc()=>tmp_slab == NULL\n");
			return NULL;
		}

		memset(slab, 0, sizeof(struct Slab));

		list_init(&slab->list);

		slab->page = alloc_pages(ZONE_NORMAL, 1, 0);

		if (slab->page == NULL) {
			color_printk(RED, BLACK, "slab_malloc()->alloc_pages()=>tmp_slab->page == NULL\n");
			kfree(slab);
			return NULL;
		}

		page_init(slab->page, PG_Kernel);

		slab->using_count = 0;
		slab->free_count = PAGE_2M_SIZE / slab_cache->size;
		slab->Vaddress = Phy_To_Virt(slab->page->PHY_address);

		slab->color_count = slab->free_count;
		slab->color_length = ALIGN(ALIGN(slab->color_count , 8) / 8, sizeof(unsigned long));
		slab->color_map = (unsigned long *)kmalloc(slab->color_length, 0);

		if (slab->color_map == NULL) {
			color_printk(RED, BLACK, "slab_malloc()->kmalloc()=>tmp_slab->color_map == NULL\n");
			free_pages(slab->page, 1);
			kfree(slab);
			return NULL;
		}

		memset(slab->color_map, 0, slab->color_length);
		list_add_to_behind(&slab_cache->cache_pool->list, &slab->list);

		slab_cache->total_free  += slab->color_count;

	} else {
		while (1) {
			if (slab->free_count == 0) {
				slab = container_of(list_next(&slab->list), struct Slab, list);
				continue;
			} else
				break;
			if (slab == slab_cache->cache_pool) {
				color_printk(BLUE, BLACK, "slab_malloc() ERROR: slab_cache->total_free != 0\n");
				return NULL;
			}
		}
		
	}

	for (int j = 0; j < slab->color_count; j++) {
		if (slab->color_map[j / BITS_PER_LONG] == 0xffffffffffffffffUL) {
			j += BITS_PER_LONG - 1;
			continue;
		}

		if ((slab->color_map[j / BITS_PER_LONG] & (1UL << (j % BITS_PER_LONG))) == 0) {
			BIT_SET(slab->color_map, j);

			slab->using_count++;
			slab->free_count--;

			slab_cache->total_using++;
			slab_cache->total_free--;

			if (slab_cache->constructor != NULL) {
				return slab_cache->constructor((char *)slab->Vaddress + slab_cache->size * j, arg);
			} else {
				return (void *)((char *)slab->Vaddress + slab_cache->size * j);
			}
		}
	}
	color_printk(RED, BLACK, "slab_malloc() ERROR: can`t alloc\n");
	if (slab != NULL && slab->using_count == 0) {
		list_del(&slab->list);
		kfree(slab->color_map);
		page_clean(slab->page);
		free_pages(slab->page, 1);
		kfree(slab);
	}

	return NULL;
}

/*

*/

unsigned long slab_free(struct Slab_cache *slab_cache, void *address, unsigned long arg) {

	struct Slab *slab = slab_cache->cache_pool;
	int index = 0;

	do {
		if (slab->Vaddress <= address && address < slab->Vaddress + PAGE_2M_SIZE) {
			index = (address - slab->Vaddress) / slab_cache->size;
			BIT_NOT(slab->color_map, index);
			slab->free_count++;
			slab->using_count--;

			slab_cache->total_using--;
			slab_cache->total_free++;

			if (slab_cache->destructor != NULL) {
				slab_cache->destructor((char *)slab->Vaddress + slab_cache->size * index, arg);
			}

			if ((slab->using_count == 0) && (slab_cache->total_free >= slab->color_count * 3 / 2)) {
				list_del(&slab->list);
				slab_cache->total_free -= slab->color_count;

				kfree(slab->color_map);

				page_clean(slab->page);
				free_pages(slab->page, 1);
				kfree(slab);
			}

			return 1;
		} else {
			slab = container_of(list_next(&slab->list), struct Slab, list);
			continue;
		}

	} while (slab != slab_cache->cache_pool);

	color_printk(RED, BLACK, "slab_free() ERROR: address not in slab\n");

	return 0;
}

/*

*/

unsigned long slab_init() {
	struct Page *page = NULL;
	unsigned long *virtual = NULL;  // get a free page and set to empty page table and return the virtual address
	unsigned long i, j;

	unsigned long tmp_address = mms.end_of_struct;

	for (i = 0; i < ARRAY_SIZE(kmalloc_cache_size); i++) {
		kmalloc_cache_size[i].cache_pool = (struct Slab *)mms.end_of_struct;
		mms.end_of_struct = mms.end_of_struct + sizeof(struct Slab) + sizeof(long) * 10;

		list_init(&kmalloc_cache_size[i].cache_pool->list);

		//////////// init sizeof struct Slab of cache size

		kmalloc_cache_size[i].cache_pool->using_count = 0;
		kmalloc_cache_size[i].cache_pool->free_count  = PAGE_2M_SIZE / kmalloc_cache_size[i].size;

		kmalloc_cache_size[i].cache_pool->color_length = ALIGN(ALIGN(PAGE_2M_SIZE / kmalloc_cache_size[i].size, 8) / 8, sizeof(long));
		kmalloc_cache_size[i].cache_pool->color_count = kmalloc_cache_size[i].cache_pool->free_count;
		kmalloc_cache_size[i].cache_pool->color_map = (unsigned long *)mms.end_of_struct;

		mms.end_of_struct = ALIGN(mms.end_of_struct + kmalloc_cache_size[i].cache_pool->color_length + sizeof(long) * 10, sizeof(long));

		memset(kmalloc_cache_size[i].cache_pool->color_map, 0xff, kmalloc_cache_size[i].cache_pool->color_length);

		for (j = 0; j < kmalloc_cache_size[i].cache_pool->color_count; j++)
			BIT_NOT(kmalloc_cache_size[i].cache_pool->color_map, j);

		kmalloc_cache_size[i].total_free = kmalloc_cache_size[i].cache_pool->color_count;
		kmalloc_cache_size[i].total_using = 0;

	}

	////////////	init page for kernel code and memory management struct

	i = PAGE_2M_NUM(Virt_To_Phy(mms.end_of_struct));

	for (j = PAGE_2M_NUM(PAGE_2M_ALIGN(Virt_To_Phy(tmp_address))); j <= i; j++) {
		page =  &mms.pages_struct[j];
		BIT_SET(mms.bits_map, PAGE_2M_NUM(page->PHY_address));
		page->zone_struct->page_using_count++;
		page->zone_struct->page_free_count--;
		page_init(page, PG_PTable_Maped | PG_Kernel_Init | PG_Kernel);
	}

	color_printk(ORANGE, BLACK, "2.mms.bits_map:%#010lx\tzone_struct->page_using_count:%d\tzone_struct->page_free_count:%d\n", *mms.bits_map, mms.zones_struct->page_using_count, mms.zones_struct->page_free_count);

	for (i = 0; i < ARRAY_SIZE(kmalloc_cache_size); i++) {
		virtual = (unsigned long *)ALIGN(mms.end_of_struct + PAGE_2M_SIZE * i, PAGE_2M_SIZE);
		page = Virt_To_2M_Page(virtual);

		BIT_SET(mms.bits_map, PAGE_2M_NUM(page->PHY_address));
		page->zone_struct->page_using_count++;
		page->zone_struct->page_free_count--;

		page_init(page, PG_PTable_Maped | PG_Kernel_Init | PG_Kernel);

		kmalloc_cache_size[i].cache_pool->page = page;
		kmalloc_cache_size[i].cache_pool->Vaddress = virtual;
	}

	color_printk(ORANGE, BLACK, "3.mms.bits_map:%#010lx\tzone_struct->page_using_count:%d\tzone_struct->page_free_count:%d\n", *mms.bits_map, mms.zones_struct->page_using_count, mms.zones_struct->page_free_count);

	color_printk(ORANGE, BLACK, "start_code:%#010lx,end_code:%#010lx,end_data:%#010lx,start_brk:%#010lx,end_of_struct:%#010lx\n", mms.start_code, mms.end_code, mms.end_data, mms.start_brk, mms.end_of_struct);

	return 1;
}


unsigned long do_brk(unsigned long addr, unsigned long len) {
	unsigned long * tmp = NULL;
	unsigned long * virtual = NULL;
	struct Page * p = NULL;
	unsigned long i = 0;

	for (i = addr; i < addr + len; i += PAGE_2M_SIZE) {
		tmp = Phy_To_Virt(pgd_offset(current->mm, addr));
		if (*tmp == NULL) {
			p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
			if (p == NULL)
				return -ENOMEM;
			pgd_t *pgd = pgd_offset(current->mm, addr);
			set_pgd(pgd, p->PHY_address, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);
		}
	}

	current->mm->end_brk = i;

	flush_tlb();

	return i;
}
