#include <asm_offset.h>
#include <ptrace.h>
#include <linkage.h>

#define S_OFF		8

ENTRY(vector_irq)
	sub	lr, lr, #4

	@
	@ Save r0, lr_<exception> (parent PC) and spsr_<exception>
	@ (parent CPSR)
	@
	stmia	sp, {r0, lr}		@ save r0, lr
	mrs	lr, spsr
	str	lr, [sp, #8]		@ save spsr


	@
	@ Prepare for SVC32 mode.  IRQs remain disabled.
	@
	mrs	r0, cpsr
	eor	r0, r0, #(IRQ_MODE ^ SVC_MODE | PSR_ISETSTATE)
	msr	spsr_cxsf, r0

	@
	@ the branch table must immediately follow this code
	@
	mov	r0, sp
	mov	lr, pc
	movs pc, lr			@ branch to handler in SVC mode

	sub	sp, sp, #(S_FRAME_SIZE - 4)
	stmia	sp, {r1 - r12}

	ldmia	r0, {r3 - r5}
	add	r7, sp, #S_SP - 4	@ here for interlock avoidance
	mov	r6, #-1			@  ""  ""      ""       ""
	add	r2, sp, #(S_FRAME_SIZE - 4)
	str	r3, [sp, #-4]!		@ save the "real" r0 copied
					@ from the exception stack

	mov	r3, lr

	@
	@ We are now ready to fill in the remaining blanks on the stack:
	@
	@  r2 - sp_svc
	@  r3 - lr_svc
	@  r4 - lr_<exception>, already fixed up for correct return/restart
	@  r5 - spsr_<exception>
	@  r6 - orig_r0 (see pt_regs definition in ptrace.h)
	@
	stmia	r7, {r2 - r6}

	mov	r0, sp

    bl	do_irq
	
	msr	spsr_cxsf, r5
	ldmia	sp, {r0 - pc}^			@ load r0 - pc, cpsr


ENTRY(vector_swi)
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0 - r12
	add	r8, sp, #S_PC
	add r9, sp, #S_FRAME_SIZE
	//虽然不影响运行，但原始的lr值已丢失
	stmdb	r8, {r9, lr}			@ Calling sp, lr
	mrs	r8, spsr			@ called from non-FIQ mode, so ok.
	str	lr, [sp, #S_PC]			@ Save calling PC
	str	r8, [sp, #S_PSR]		@ Save CPSR
	str	r0, [sp, #S_OLD_R0]		@ Save OLD_R0

	mov fp, #0
	//TODO:安全检查
	stmdb	sp!, {r4, r5}
	ldr r9, = system_call_table
	mov lr, pc
	ldr pc, [r9, r7, lsl #2]
	ldmia	sp!, {r4, r5}
	msr	spsr_cxsf, r8
	ldmia	sp, {r0 - pc}^			@ load r0 - pc, cpsr

ENTRY(ret_system_call)
	ldr r8, [sp, #S_PSR]
	msr	spsr_cxsf, r8
	ldmia	sp, {r0 - pc}^			@ load r0 - pc, cpsr

ENTRY(sys_fork_wrapper)
	add	r0, sp, #S_OFF
	b	sys_fork
ENTRY(sys_vfork_wrapper)
	add	r0, sp, #S_OFF
	b	sys_vfork
ENTRY(sys_execve_wrapper)
	add	r0, sp, #S_OFF
	b	sys_execve
