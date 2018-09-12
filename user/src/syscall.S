#include <linkage.h>
ENTRY(syscall)
	mov		r7, r0
	mov		r0, r1
	mov		r1, r2
	mov		r2, r3
	ldmia   sp, { r3-r5 }
	stmdb	sp!, { lr }
    swi		#0
	ldmia	sp!, { lr }
	mov		pc, lr
