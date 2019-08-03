/*
 *  arch/arm/include/asm/tlbflush.h
 *
 *  Copyright (C) 1999-2003 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _ASMARM_TLBFLUSH_H
#define _ASMARM_TLBFLUSH_H

#include <barrier.h>
#include <mm_types.h>



#define TLB_V3_PAGE	(1 << 0)
#define TLB_V4_U_PAGE	(1 << 1)
#define TLB_V4_D_PAGE	(1 << 2)
#define TLB_V4_I_PAGE	(1 << 3)
#define TLB_V6_U_PAGE	(1 << 4)
#define TLB_V6_D_PAGE	(1 << 5)
#define TLB_V6_I_PAGE	(1 << 6)

#define TLB_V3_FULL	(1 << 8)
#define TLB_V4_U_FULL	(1 << 9)
#define TLB_V4_D_FULL	(1 << 10)
#define TLB_V4_I_FULL	(1 << 11)
#define TLB_V6_U_FULL	(1 << 12)
#define TLB_V6_D_FULL	(1 << 13)
#define TLB_V6_I_FULL	(1 << 14)

#define TLB_V6_U_ASID	(1 << 16)
#define TLB_V6_D_ASID	(1 << 17)
#define TLB_V6_I_ASID	(1 << 18)

#define TLB_V6_BP	(1 << 19)

/* Unified Inner Shareable TLB operations (ARMv7 MP extensions) */
#define TLB_V7_UIS_PAGE	(1 << 20)
#define TLB_V7_UIS_FULL (1 << 21)
#define TLB_V7_UIS_ASID (1 << 22)
#define TLB_V7_UIS_BP	(1 << 23)

#define TLB_BARRIER	(1 << 28)
#define TLB_L2CLEAN_FR	(1 << 29)		/* Feroceon */
#define TLB_DCLEAN	(1 << 30)
#define TLB_WB		(1 << 31)

/*
 *	MMU TLB Model
 *	=============
 *
 *	We have the following to choose from:
 *	  v3    - ARMv3
 *	  v4    - ARMv4 without write buffer
 *	  v4wb  - ARMv4 with write buffer without I TLB flush entry instruction
 *	  v4wbi - ARMv4 with write buffer with I TLB flush entry instruction
 *	  fr    - Feroceon (v4wbi with non-outer-cacheable page table walks)
 *	  fa    - Faraday (v4 with write buffer with UTLB)
 *	  v6wbi - ARMv6 with write buffer with I TLB flush entry instruction
 *	  v7wbi - identical to v6wbi
 */

#define v4wb_tlb_flags	(TLB_WB | TLB_DCLEAN | \
			TLB_V4_I_FULL | TLB_V4_D_FULL | \
			TLB_V4_D_PAGE)

#ifdef CONFIG_CPU_TLB_V4WB
# define v4wb_possible_flags	v4wb_tlb_flags
# define v4wb_always_flags	v4wb_tlb_flags
# ifdef _TLB
#  define MULTI_TLB 1
# else
#  define _TLB v4wb
# endif
#else
# define v4wb_possible_flags	0
# define v4wb_always_flags	(-1UL)
#endif

#define v7wbi_tlb_flags_smp	(TLB_WB | TLB_DCLEAN | TLB_BARRIER | \
			TLB_V7_UIS_FULL | TLB_V7_UIS_PAGE | \
			TLB_V7_UIS_ASID | TLB_V7_UIS_BP)
#define v7wbi_tlb_flags_up	(TLB_WB | TLB_DCLEAN | TLB_BARRIER | \
			TLB_V6_U_FULL | TLB_V6_U_PAGE | \
			TLB_V6_U_ASID | TLB_V6_BP)

#ifdef CONFIG_CPU_TLB_V7

# ifdef CONFIG_SMP_ON_UP
#  define v7wbi_possible_flags	(v7wbi_tlb_flags_smp | v7wbi_tlb_flags_up)
#  define v7wbi_always_flags	(v7wbi_tlb_flags_smp & v7wbi_tlb_flags_up)
# elif defined(CONFIG_SMP)
#  define v7wbi_possible_flags	v7wbi_tlb_flags_smp
#  define v7wbi_always_flags	v7wbi_tlb_flags_smp
# else
#  define v7wbi_possible_flags	v7wbi_tlb_flags_up
#  define v7wbi_always_flags	v7wbi_tlb_flags_up
# endif
# ifdef _TLB
#  define MULTI_TLB 1
# else
#  define _TLB v7wbi
# endif
#else
# define v7wbi_possible_flags	0
# define v7wbi_always_flags	(-1UL)
#endif

#ifndef _TLB
#error Unknown TLB model
#endif

#ifndef __ASSEMBLY__

/*
 *	TLB Management
 *	==============
 *
 *	The arch/arm/mm/tlb-*.S files implement these methods.
 *
 *	The TLB specific code is expected to perform whatever tests it
 *	needs to determine if it should invalidate the TLB for each
 *	call.  Start addresses are inclusive and end addresses are
 *	exclusive; it is safe to round these addresses down.
 *
 *	flush_tlb_all()
 *
 *		Invalidate the entire TLB.
 *
 *	flush_tlb_mm(mm)
 *
 *		Invalidate all TLB entries in a particular address
 *		space.
 *		- mm	- mm_struct describing address space
 *
 *	flush_tlb_range(mm,start,end)
 *
 *		Invalidate a range of TLB entries in the specified
 *		address space.
 *		- mm	- mm_struct describing address space
 *		- start - start address (may not be aligned)
 *		- end	- end address (exclusive, may not be aligned)
 *
 *	flush_tlb_page(vaddr,vma)
 *
 *		Invalidate the specified page in the specified address range.
 *		- vaddr - virtual address (may not be aligned)
 *		- vma	- vma_struct describing address range
 *
 *	flush_kern_tlb_page(kaddr)
 *
 *		Invalidate the TLB entry for the specified page.  The address
 *		will be in the kernels virtual memory space.  Current uses
 *		only require the D-TLB to be invalidated.
 *		- kaddr - Kernel virtual memory address
 */

/*
 * We optimise the code below by:
 *  - building a set of TLB flags that might be set in __cpu_tlb_flags
 *  - building a set of TLB flags that will always be set in __cpu_tlb_flags
 *  - if we're going to need __cpu_tlb_flags, access it once and only once
 *
 * This allows us to build optimal assembly for the single-CPU type case,
 * and as close to optimal given the compiler constrants for multi-CPU
 * case.  We could do better for the multi-CPU case if the compiler
 * implemented the "%?" method, but this has been discontinued due to too
 * many people getting it wrong.
 */

#define always_tlb_flags	(v4wb_always_flags & \
				v7wbi_always_flags)

#define tlb_flag(f)	(always_tlb_flags & (f))

#define __tlb_op(f, insnarg, arg)					\
	do {								\
		if (always_tlb_flags & (f))				\
			asm("mcr " insnarg	: : "r" (arg) : "cc");			\
	} while (0)

#define tlb_op(f, regs, arg)	__tlb_op(f, "p15, 0, %0, " regs, arg)
#define tlb_l2_op(f, regs, arg)	__tlb_op(f, "p15, 1, %0, " regs, arg)

static inline void local_flush_tlb_all(void)
{
	const int zero = 0;

	if (tlb_flag(TLB_WB))
		dsb();

	tlb_op(TLB_V4_D_FULL, "c8, c6, 0", zero);
	tlb_op(TLB_V4_I_FULL, "c8, c5, 0", zero);
	tlb_op(TLB_V7_UIS_FULL, "c8, c3, 0", zero);

	if (tlb_flag(TLB_BARRIER)) {
		dsb();
		isb();
	}
}

static inline void local_flush_tlb_mm(struct mm_struct *mm)
{

}

static inline void
local_flush_tlb_page(struct vm_area_struct *vma, unsigned long uaddr)
{
}

static inline void local_flush_tlb_kernel_page(unsigned long kaddr)
{
}

/*
 *	flush_pmd_entry
 *
 *	Flush a PMD entry (word aligned, or double-word aligned) to
 *	RAM if the TLB for the CPU we are running on requires this.
 *	This is typically used when we are creating PMD entries.
 *
 *	clean_pmd_entry
 *
 *	Clean (but don't drain the write buffer) if the CPU requires
 *	these operations.  This is typically used when we are removing
 *	PMD entries.
 */
static inline void flush_pmd_entry(void *pmd)
{
	tlb_op(TLB_DCLEAN, "c7, c10, 1	@ flush_pmd", pmd);
	tlb_l2_op(TLB_L2CLEAN_FR, "c15, c9, 1  @ L2 flush_pmd", pmd);

	if (tlb_flag(TLB_WB))
		dsb();
}

static inline void clean_pmd_entry(void *pmd)
{
	tlb_op(TLB_DCLEAN, "c7, c10, 1	@ flush_pmd", pmd);
	tlb_l2_op(TLB_L2CLEAN_FR, "c15, c9, 1  @ L2 flush_pmd", pmd);
}

#undef tlb_op
#undef tlb_flag
#undef always_tlb_flags

/*
 * Convert calls to our calling convention.
 */

#define flush_tlb_all		local_flush_tlb_all


#endif

#endif
