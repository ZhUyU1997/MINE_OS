#include <asm_offset.h>
#include <ptrace.h>
#include <linkage.h>
.text
ENTRY(_start)      
    b   Reset
@ 0x04: 未定义指令中止模式的向量地址
    ldr pc, = HandleUndef 
@ 0x08: 管理模式的向量地址，通过SWI指令进入此模式
    ldr pc, = vector_swi
@ 0x0c: 指令预取终止导致的异常的向量地址
    ldr pc, = HandlePrefetchAbort
@ 0x10: 数据访问终止导致的异常的向量地址
    ldr pc, = HandleDataAbort
@ 0x14: 保留
HandleNotUsed:
    ldr pc, = HandleNotUsed
@ 0x18: 中断模式的向量地址
    ldr pc, = vector_irq
@ 0x1c: 快中断模式的向量地址
HandleFIQ:
    ldr pc, = HandleFIQ

Reset:
#if defined(CONFIG_BOOT_FORM_STEPPINGSTONE)
	msr cpsr_c, #0xd3		@ 进入管理模式设置I-bit=1，关IRQ中断
    ldr sp, =4096           @ 设置管理模式栈指针
#elif defined(CONFIG_BOOT_FORM_SDRAM)
	msr cpsr_c, #0xd3		@ 进入管理模式设置I-bit=1，关IRQ中断
    ldr sp, =0x33f00000     @ 设置管理模式栈指针
#else
	#error 请设置启动模式
#endif
	bl disable_watch_dog	@ 1. 关看门狗
	bl clock_init			@ 2. 设置时钟
	bl memsetup				@ 3. 初始化SDRAM

#ifdef CONFIG_BOOT_FORM_STEPPINGSTONE
	bl nand_init_ll

	@复制代码到SDRAM中					
    ldr r0, =_start			@ 1. 目标地址
    mov r1, #0				@ 2. 源地址  运行地址在SDRAM中的代码保存在NAND Flash 0地址开始处
	ldr r2, =_bss
	sub r2, r2, r0			@ 3. 复制长度 = __bss_start - _start
    bl  copy_code_to_sdram	@ 调用C函数copy_code_to_SDRAM
#endif
    bl  clean_bss			@ 清除bss段，未初始化或初值为0的全局/静态变量保存在bss段
    msr cpsr_c, #0xd2		@ 进入中断模式,设置中断模式栈指针
    ldr sp, =CONFIG_IRQ_STACK_BASE_ADDR

    msr cpsr_c, #0xdf		@ 进入系统模式,设置系统模式栈指针
    ldr sp, =CONFIG_SYS_STACK_BASE_ADDR

	msr cpsr_c, #0xd7		@ 进入数据访问终止模式,设置数据访问终止模式栈指针
    ldr sp, =CONFIG_ABT_STACK_BASE_ADDR

	msr cpsr_c, #0xdb		@ 进入未定义指令中止模式,设置未定义指令中止模式栈指针
    ldr sp, =CONFIG_UND_STACK_BASE_ADDR

	msr cpsr_c, #0xd3		@ 进入管理模式设置I-bit=1，关IRQ中断,设置管理模式栈指针
    ldr sp, =CONFIG_SWI_STACK_BASE_ADDR
	ldr fp, =0				@ 设置栈帧指针

    ldr lr, =halt_loop		@ 设置返回地址
    ldr pc, =main			@ 调用中断初始化函数
halt_loop:
    b   halt_loop
	
