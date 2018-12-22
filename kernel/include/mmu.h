#ifndef __MMU_H__
#define  __MMU_H__

#include <sys/types.h>

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

void set_pmd(pmd_t *pmd, u32_t physicaladdr, u32_t AP, u32_t domain, u32_t C, u32_t B);
void set_pgd(pgd_t *pgd, u32_t physicaladdr, u32_t AP, u32_t domain, u32_t C, u32_t B);
void mmu_init(void);

#endif
