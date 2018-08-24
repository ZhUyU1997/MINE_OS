#include <linkage.h>

/*
 * The size of one data cache line.
 */
#define CACHE_DLINESIZE	32

/*
 * The number of data cache segments.
 */
#define CACHE_DSEGMENTS	8

/*
 * The number of lines in a cache segment.
 */
#define CACHE_DENTRIES	64

ENTRY(cpu_arm920_switch_mm)
	mov	ip, #0
@ && 'Clean & Invalidate whole DCache'
@ && Re-written to use Index Ops.
@ && Uses registers r1, r3 and ip

	mov	r1, #(CACHE_DSEGMENTS - 1) << 5	@ 8 segments
1:	orr	r3, r1, #(CACHE_DENTRIES - 1) << 26 @ 64 entries
2:	mcr	p15, 0, r3, c7, c14, 2		@ clean & invalidate D index
	subs	r3, r3, #1 << 26
	bcs	2b				@ entries 63 to 0
	subs	r1, r1, #1 << 5
	bcs	1b				@ segments 7 to 0

	mcr	p15, 0, ip, c7, c5, 0		@ invalidate I cache
	mcr	p15, 0, ip, c7, c10, 4		@ drain WB
	mcr	p15, 0, r0, c2, c0, 0		@ load page table pointer
	mcr	p15, 0, ip, c8, c7, 0		@ invalidate I & D TLBs

	mov	pc, lr
	
/*
 *	flush_kern_cache_all()
 *
 *	Clean and invalidate the entire cache.
 */
ENTRY(arm920_flush_kern_cache_all)
	mov	ip, #0
__flush_whole_cache:
	mov	r1, #(CACHE_DSEGMENTS - 1) << 5	@ 8 segments
1:	orr	r3, r1, #(CACHE_DENTRIES - 1) << 26 @ 64 entries
2:	mcr	p15, 0, r3, c7, c14, 2		@ clean+invalidate D index
	subs	r3, r3, #1 << 26
	bcs	2b				@ entries 63 to 0
	subs	r1, r1, #1 << 5
	bcs	1b				@ segments 7 to 0
	mcr	p15, 0, ip, c7, c5, 0		@ invalidate I cache
	mcr	p15, 0, ip, c7, c10, 4		@ drain WB
	mov	pc, lr
