#ifndef __ARM32_MMU_H__
#define __ARM32_MMU_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <core/machine.h>
#include <pgtable.h>

enum {
	MAP_TYPE_NCNB	= 0x0,
	MAP_TYPE_NCB	= 0x1,
	MAP_TYPE_CNB	= 0x2,
	MAP_TYPE_CB		= 0x3,
};


#define MMU_FULL_ACCESS     (3)			/* 访问权限 */
#define MMU_DOMAIN(x)       (x)			/* 属于哪个域 */
#define MMU_SPECIAL         (1)			/* 必须是1 */

#define MMU_PAGE_TABLE     (1)         /* 表示这是粗页描述符 */
#define MMU_SECTION         (2)         /* 表示这是段描述符 */

#define MMU_LARGE_PAGE 	    (1)         /* 表示这是大页描述符 */
#define MMU_SMALL_PAGE		(2)         /* 表示这是小页描述符 */

#define MMU_PAGE_TABLE_SHIFT		(10)
#define MMU_SECTION_SHIFT			(20)

#define MMU_LARGE_PAGE_SHIFT 	    (16)
#define MMU_SMALL_PAGE_SHIFT		(12)

#define MMU_SECTION_SIZE	(1<<MMU_SECTION_SHIFT)
#define MMU_LARGE_PAGE_SIZE	(1<<MMU_LARGE_PAGE_SHIFT)
#define MMU_SMALL_PAGE_SIZE	(1<<MMU_SMALL_PAGE_SHIFT)


typedef struct COARSE_PAGE {
	u32_t type: 2;
	u32_t blank0: 1;
	u32_t NS: 1;
	u32_t blank1: 1;
	u32_t domain: 4;
	u32_t IMP: 1;
	u32_t base_address: 22;
} PAGE_TABLE;

typedef struct SECTION {
	u32_t type: 2;
	u32_t CB: 2;
	u32_t XN: 1;
	u32_t domain: 4;
	u32_t IMP: 1;
	u32_t AP: 2;
	u32_t TEX: 3;
	u32_t AP2: 1;
	u32_t S: 1;
	u32_t nG: 1;
	u32_t zero: 1;
	u32_t NS: 1;
	u32_t base_address: 12;
} SECTION;

typedef struct SMALL_PAGE {
	u32_t XN: 1;
	u32_t one: 1;
	u32_t CB: 2;
	u32_t AP: 2;
	u32_t TEX: 3;
	u32_t AP2: 1;
	u32_t S: 1;
	u32_t nG: 1;
	u32_t base_address: 20;
} SMALL_PAGE;

void set_pte(pte_t *pte, u32_t physicaladdr, u32_t AP, u32_t map_type);
void set_pmd(pmd_t *pmd, u32_t physicaladdr, u32_t AP, u32_t map_type);
void * mmu_get_tlb_base_addr();
void mmu_setup(struct machine_t * mach);

#ifdef __cplusplus
}
#endif

#endif /* __ARM32_MMU_H__ */
