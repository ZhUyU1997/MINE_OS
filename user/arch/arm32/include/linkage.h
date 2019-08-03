#ifndef _LINKAGE_H_
#define _LINKAGE_H_

#define L1_CACHE_BYTES 32
#define asmlinkage __attribute__((regparm(0)))
#define ____cacheline_aligned __attribute__((__aligned__(L1_CACHE_BYTES)))

#define __ALIGN .align 0
#define __ALIGN_STR ".align 0"

#define ALIGN __ALIGN
#define ALIGN_STR __ALIGN_STR

#ifndef ENTRY
#define ENTRY(name) \
	.globl name; \
	ALIGN; \
	name:
#endif

#ifndef WEAK
#define WEAK(name)	   \
	.weak name;	   \
	name:
#endif

#ifndef END
#define END(name) \
	.size name, .-name
#endif

/* If symbol 'name' is treated as a subroutine (gets called, and returns)
 * then please use ENDPROC to mark 'name' as STT_FUNC for the benefit of
 * static analysis tools such as stack depth analyzer.
 */

#define ENDPROC(name) \
	.type name, %function; \
	END(name)

#define ARM(x...)	x
#define THUMB(x...)

#endif
