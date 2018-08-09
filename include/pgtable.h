#ifndef _ASMARM_PGTABLE_H
#define _ASMARM_PGTABLE_H

#include <pgtable_2level.h>
#include <tlbflush.h>
#include <sys/types.h>

typedef u32 pteval_t;
typedef u32 pmdval_t;

typedef pteval_t pte_t;
typedef pmdval_t pmd_t;
typedef pmdval_t pgd_t[2];
typedef pteval_t pgprot_t;

typedef u32 phys_addr_t;

static inline void set_pte_ext(pte_t *ptep, pte_t pte){
	*ptep = pte;
	flush_pmd_entry(ptep);
}

#define ARCH_PFN_OFFSET 0

#define pfn_to_page(pfn)	(mem_map + ((pfn) - ARCH_PFN_OFFSET))
#define page_to_pfn(page)	((unsigned long)((page) - mem_map) + ARCH_PFN_OFFSET)

#define	__phys_to_pfn(paddr)	((unsigned long)((paddr) >> PAGE_SHIFT))
#define	__pfn_to_phys(pfn)	((phys_addr_t)(pfn) << PAGE_SHIFT)


#define pfn_pte(pfn,prot)	(__pfn_to_phys(pfn) | (prot))

/* to find an entry in a page-table-directory */
#define pgd_index(addr)			((addr) >> PGDIR_SHIFT)
#define pgd_offset(mm, addr)	((mm)->pgd + pgd_index(addr))

#define pte_index(addr)			(((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#define mk_pte(page,prot)		pfn_pte(page_to_pfn(page), prot)
#define pte_clear(mm,addr,ptep)	set_pte_ext(ptep, 0, 0)

#endif
