#include <global_config.h>
#include <asm_offset.h>
#include <ptrace.h>
.text
.global _start 
_start:
@******************************************************************************       
@ 中断向量，本程序中，除Reset和HandleIRQ外，其它异常都没有使用
@******************************************************************************       
    b   Reset

@ 0x04: 未定义指令中止模式的向量地址
    ldr pc, = HandleUndef 

@ 0x08: 管理模式的向量地址，通过SWI指令进入此模式
    ldr pc, = HandleSWI

@ 0x0c: 指令预取终止导致的异常的向量地址
    ldr pc, = HandlePrefetchAbort

@ 0x10: 数据访问终止导致的异常的向量地址
    ldr pc, = HandleDataAbort

@ 0x14: 保留
HandleNotUsed:
    ldr pc, = HandleNotUsed

@ 0x18: 中断模式的向量地址
    ldr pc, = HandleIRQ

@ 0x1c: 快中断模式的向量地址
HandleFIQ:
    ldr pc, = HandleFIQ

Reset:
#if BOOT_MODE == BOOT_FORM_STEPPINGSTONE
	msr cpsr_c, #0xd3		@ 进入管理模式设置I-bit=1，关IRQ中断
    ldr sp, =4096           @ 设置管理模式栈指针
#elif BOOT_MODE == BOOT_FORM_SDRAM
	msr cpsr_c, #0xd3		@ 进入管理模式设置I-bit=1，关IRQ中断
    ldr sp, =0x33f00000     @ 设置管理模式栈指针
#else
	#error 请设置启动模式
#endif
	bl disable_watch_dog	@ 1. 关看门狗
	bl clock_init			@ 2. 设置时钟
	bl memsetup				@ 3. 初始化SDRAM

#if BOOT_MODE == BOOT_FORM_STEPPINGSTONE
	bl nand_init_ll

	@复制代码到SDRAM中					
    ldr r0, =_start			@ 1. 目标地址
    mov r1, #0				@ 2. 源地址  运行地址在SDRAM中的代码保存在NAND Flash 0地址开始处
	ldr r2, =__bss_start
	sub r2, r2, r0			@ 3. 复制长度 = __bss_start - _start
    bl  copy_code_to_sdram	@ 调用C函数copy_code_to_SDRAM
#endif
    bl  clean_bss			@ 清除bss段，未初始化或初值为0的全局/静态变量保存在bss段
    msr cpsr_c, #0xd2		@ 进入中断模式
    ldr sp, =0x33effff4		@ 设置中断模式栈指针

    msr cpsr_c, #0xdf		@ 进入系统模式
    ldr sp, =0x33e00000		@ 设置系统模式栈指针

	msr cpsr_c, #0xd7		@ 进入数据访问终止模式
    ldr sp, =0x33e80000		@ 设置数据访问终止模式栈指针

	msr cpsr_c, #0xdb		@ 进入未定义指令中止模式
    ldr sp, =0x33e80000		@ 设置未定义指令中止模式栈指针

	msr cpsr_c, #0xd3		@ 进入管理模式设置I-bit=1，关IRQ中断
    ldr sp, =0x33effff4		@ 设置管理模式栈指针
	ldr fp, =0x33effff4		@ 设置栈帧指针

    ldr lr, =halt_loop		@ 设置返回地址
    ldr pc, =main			@ 调用中断初始化函数
halt_loop:
    b   halt_loop

HandleIRQ:
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

HandleSWI:
	b HandleSWI
