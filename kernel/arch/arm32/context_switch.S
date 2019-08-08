#include <linkage.h>
ENTRY(__switch_to)
	mov ip, r0
	stmia	ip!, {r4 - sl, fp, sp, lr}
	vstmia  ip!, {d8 - d15}
	mov r4, r1
	vldmdb  r4!, {d8 - d15}
	ldmdb	r4, {r4 - sl, fp, sp, pc}
