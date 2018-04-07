@******************************************************************************
@ File: head.S
@ 功能: 设置SDRAM，将程序复制到SDRAM，然后跳到SDRAM继续执行
@******************************************************************************       
#define S3C2440_MPLL_200MHZ     ((0x5c<<12)|(0x01<<4)|(0x02))
#define S3C2440_MPLL_400MHZ     ((0x5c<<12)|(0x01<<4)|(0x01))
#define MEM_CTL_BASE    0x48000000
.extern     main
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
    b   HandleSWI

@ 0x0c: 指令预取终止导致的异常的向量地址
    ldr pc, = HandlePrefetchAbort

@ 0x10: 数据访问终止导致的异常的向量地址
    ldr pc, = HandleDataAbort

@ 0x14: 保留
HandleNotUsed:
    b   HandleNotUsed

@ 0x18: 中断模式的向量地址
    b   HandleIRQ

@ 0x1c: 快中断模式的向量地址
HandleFIQ:
    b   HandleFIQ

Reset:
	ldr sp, =0x4096
/* 1. 关看门狗 */
	bl disable_watch_dog

/* 2. 设置时钟 */
	bl clock_init

	/* 启动ICACHE */
	bl enable_ICACNE
	
	
/* 3. 初始化SDRAM */
	bl memsetup
		
    ldr sp, =0x33f80000     @ 设置管理模式栈指针
	
	bl nand_init_ll
	
	@ 复制代码到SDRAM中					
    ldr r0, =_start			@ 1. 目标地址 = 0x30000000，这是SDRAM的起始地址
    mov r1, #0				@ 2. 源地址   = 4096，运行地址在SDRAM中的代码保存在NAND Flash 4096地址开始处
	ldr r2, =__bss_start
	sub r2, r2, r0			@ 3. 复制长度 = __bss_start - _start
    bl  copy_code_to_sdram  @ 调用C函数copy_code_to_SDRAM
	
    bl  clean_bss           @ 清除bss段，未初始化或初值为0的全局/静态变量保存在bss段
	
    msr cpsr_c, #0xd2       @ 进入中断模式
    ldr sp, =0x33d00000     @ 设置中断模式栈指针

    msr cpsr_c, #0xdf       @ 进入系统模式
    ldr sp, =0x33e00000     @ 设置系统模式栈指针
	
	msr cpsr_c, #0xd7       @ 进入数据访问终止模式
    ldr sp, =0x33e80000     @ 设置数据访问终止模式栈指针
	
	msr cpsr_c, #0xdb       @ 进入未定义指令中止模式
    ldr sp, =0x33f00000     @ 设置未定义指令中止模式栈指针

	msr cpsr_c, #0xd3		@ 进入管理模式设置I-bit=0，开IRQ中断
    ldr lr, =ret_initirq    @ 设置返回地址
    ldr pc, =init_irq       @ 调用中断初始化函数
ret_initirq:
    
	
	@bl  create_page_table	@ 设置页表
    @bl  mmu_init			@ 启动MMU
	
	
	@跳转地址过大不能用bl指令
    ldr lr, =halt_loop      @ 设置返回地址
    ldr pc, =main           @ 调用main函数
halt_loop:
    b   halt_loop
/*
HandleIRQ:
	sub lr, lr, #4                  @ 计算返回地址
    stmdb   sp!,    { r0-r12,lr }   @ 保存使用到的寄存器

                                    @ 注意，此时的sp是中断模式的sp
                                    @ 初始值是上面设置的0x33d00000

    LDR		LR, =int_return			@ 设置调用IRQ_Handle函数后的返回地址  
    LDR		PC, =IRQ_Handle			@ 调用中断分发函数，在interrupt.c中
int_return:
	ldmia   sp!,    { r0-r12,pc }^  @ 中断返回, ^表示将spsr的值复制到cpsr
*/
HandleIRQ:
	STMFD   SP, {R0-R3}
	MOV     R1, SP
	SUB     R2, LR, #4              @ Adjust PC for return address to task
	MRS     R3, SPSR				@ Copy SPSR (Task CPSR)
	
	@切换到SVC模式下，加载SP,LR
	MRS		R0, CPSR
	MSR     CPSR_c, #0xd3			@ Change to SVC mode
	STMFD   SP!, {R2}				@ Push task''s PC 
	STMFD   SP!, {R4-R12, LR}		@ Push task''s LR,R12-R4
	LDMDB   R1!, {R4-R7}			@ Load Task''s R0-R3 from IRQ stack 
	STMFD   SP!, {R4-R7}			@ Push Task''s R0-R3 to SVC stack
	STMFD   SP!, {R3}				@ Push task''s CPSR

	MSR		CPSR_cxsf, R0			@切换回当前模式

                                    @ 注意，此时的sp是中断模式的sp
                                    @ 初始值是上面设置的0x33d00000

    LDR		LR, =int_return			@ 设置调用IRQ_Handle函数后的返回地址  
    LDR		PC, =IRQ_Handle			@ 调用中断分发函数，在interrupt.c中
int_return:
	MSR     CPSR_c, #0xd3
	LDMFD   SP!, {R0}               @ POP the task''s CPSR 
    MSR		SPSR_cxsf, R0
    LDMFD   SP!, {R0-R12,LR,PC}^	@ 中断返回, ^表示将spsr的值复制到cpsr


HandleSWI:
	b HandleSWI
