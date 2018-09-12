#ifndef _MM_H
#define _MM_H

#define _MEM_END	0x33c00000
#define _MEM_START	0x30100000//1M对齐


#define PAGE_SHIFT	(12)
#define PAGE_SIZE	(1<<PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#define KERNEL_MEM_END	(_MEM_END)
/*the bigin and end of the kernel mem which is needed to be paged.*/
#define KERNEL_PAGING_START	((_MEM_START+(~PAGE_MASK))&((PAGE_MASK)))
#define	KERNEL_PAGING_END	(((KERNEL_MEM_END-KERNEL_PAGING_START)/(PAGE_SIZE+sizeof(struct page)))*(PAGE_SIZE)+KERNEL_PAGING_START)

/*page number in need */
#define KERNEL_PAGE_NUM	((KERNEL_PAGING_END-KERNEL_PAGING_START)/PAGE_SIZE)
/*the start and end of the page structure should be storaged in.*/
#define KERNEL_PAGE_END	_MEM_END
#define KERNEL_PAGE_START	(KERNEL_PAGE_END-KERNEL_PAGE_NUM*sizeof(struct page))

/*page size*/
#define PAGE_4K		(1024*4)
#define PAGE_8K		(1024*8)
#define PAGE_16K	(1024*16)
#define PAGE_32K	(1024*32)
#define PAGE_64K	(1024*64)
#define PAGE_128K	(1024*128)
#define PAGE_256K	(1024*256)
#define PAGE_512K	(1024*512)
#define PAGE_1M		(1024*1024)

/*page flags*/
#define PAGE_AVAILABLE		0x00
#define PAGE_DIRTY			0x01
#define PAGE_PROTECT		0x02
#define PAGE_BUDDY_BUSY		0x04
#define PAGE_IN_CACHE		0x08

/*page align flags*/
#define PAGE_ALIGN_SHIFT	(4)
#define PAGE_ALIGN_MASK		(0xff<<PAGE_ALIGN_SHIFT)//用于页面对齐

#define PAGE_ALIGN_4K	(0<<4)
#define PAGE_ALIGN_8K	(1<<4)
#define PAGE_ALIGN_16K	(2<<4)
#define PAGE_ALIGN_32K	(3<<4)
#define PAGE_ALIGN_64K	(4<<4)
#define PAGE_ALIGN_128K	(5<<4)
#define PAGE_ALIGN_256K	(6<<4)
#define PAGE_ALIGN_512K	(7<<4)
#define PAGE_ALIGN_1M	(8<<4)

#define PAGE_ORDER_4K	(0)
#define PAGE_ORDER_8K	(1)
#define PAGE_ORDER_16K	(2)
#define PAGE_ORDER_32K	(3)
#define PAGE_ORDER_64K	(4)
#define PAGE_ORDER_128K	(5)
#define PAGE_ORDER_256K	(6)
#define PAGE_ORDER_512K	(7)
#define PAGE_ORDER_1M	(8)

typedef unsigned long phys_t;
typedef unsigned long virt_t;


extern void *kmalloc(unsigned int size);
extern void kfree(void *addr);

extern void *get_free_pages(unsigned int flags, int order);
extern void put_free_pages(void *addr, int order);


// extern phys_t unmap_page(virt_t line, long pdtr);
// extern void map_page(virt_t va, phys_t pa, long pdtr);
// extern long copy_mm(void);
// extern void alloc_mm(long pdtr, long addr, long size);
// extern void free_mm(void);

#endif
