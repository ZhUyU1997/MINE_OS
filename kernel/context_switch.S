#include <linkage.h>
ENTRY(__switch_to)
	mov ip, r0
	stmia	ip!, {r4 - sl, fp, sp, lr}
	mov r4, r1
	ldmia	r4, {r4 - sl, fp, sp, pc}
