/***************************************************
*		版权声明
*
*	本操作系统名为：MINE
*	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
*	只允许个人学习以及公开交流使用
*
*	代码最终所有权及解释权归田宇所有；
*
*	本模块作者：	田宇
*	EMail:		345538255@qq.com
*
*
***************************************************/
#include "ldscript.h"
#include "task.h"
#include "ptrace.h"
#include "printk.h"
#include "lib.h"
#include "memory.h"
#include "linkage.h"
#include "schedule.h"
#include "sched.h"
#include "timer.h"
#include "smp.h"
#include "assert.h"

struct mm_struct init_mm = {0};

struct thread_struct init_thread = {
	.trap_nr = 0,
	.error_code = 0
};

union task_union init_task_union = {INIT_TASK(init_task_union.task)};
struct task_struct *init_task[NR_CPUS] = {&init_task_union.task,0};
struct task_struct *current = NULL;

system_call_t system_call_table[MAX_SYSTEM_CALL_NR] = 
{
	[0] = no_system_call,
	[1] = sys_printf,
	[2 ... MAX_SYSTEM_CALL_NR-1] = no_system_call
};

unsigned long no_system_call(struct pt_regs * regs)
{
	return -1;
}

unsigned long sys_printf(struct pt_regs * regs)
{
	return 1;
}

unsigned long do_execve(struct pt_regs * regs)
{
	return 1;
}

unsigned long init(unsigned long arg)
{
	struct pt_regs *regs;

	color_printk(GREEN,BLACK,"init task is running,arg:%#018lx\n",arg);
	return 1;
}

unsigned long do_fork(struct pt_regs *regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size) {
	struct task_struct *tsk = NULL;
	struct thread_struct *thd = NULL;

	tsk = kmalloc(STACK_SIZE, 0);
	color_printk(WHITE, BLACK, "struct task_struct address:%#018lx\n", (unsigned long)tsk);
	memset(tsk, 0, sizeof(struct task_struct));

	*tsk = *current;

	list_init(&tsk->list);

	tsk->priority = 2;
	tsk->pid++;
	tsk->state = TASK_UNINTERRUPTIBLE;
	tsk->vrun_time = 0;
	tsk->cpu_id = SMP_cpu_id();
	regs->ARM_sp = (unsigned long)tsk + STACK_SIZE  - sizeof(struct pt_regs);
	memcpy((void *)((unsigned long)tsk + STACK_SIZE - sizeof(struct pt_regs)), regs, sizeof(struct pt_regs));
	memset(&tsk->cpu_context, 0, sizeof(struct cpu_context_save));

	tsk->cpu_context.pc = regs->ARM_pc;
	tsk->cpu_context.sp = regs->ARM_sp;
	tsk->state = TASK_RUNNING;
	insert_task_queue(tsk);

	return 1;
}

unsigned long do_exit(unsigned long code) {
	color_printk(GREEN, BLACK, "exit task is running,arg:%#018lx\n", code);
	while (1);
}

extern void kernel_thread_helper(void);
asm(	".pushsection .text\n"
		"	.align\n"
		"	.type	kernel_thread_helper, #function\n"
		"kernel_thread_helper:\n"
		//TODO:
		"	ldmia sp, {r0 - r12}\n"
		"	msr	cpsr_c, r7\n"
		"	mov	r0, r4\n"
		"	mov	lr, r6\n"
		"	mov	pc, r5\n"
		"	.size	kernel_thread_helper, . - kernel_thread_helper\n"
		"	.popsection");

extern void kernel_thread_exit(long code);
asm(	".pushsection .text\n"
		"	.align\n"
		"	.type	kernel_thread_exit, #function\n"
		"kernel_thread_exit:\n"
		"	.fnstart\n"
		"	.cantunwind\n"
		"	bl	do_exit\n"
		"	nop\n"
		"	.fnend\n"
		"	.size	kernel_thread_exit, . - kernel_thread_exit\n"
		"	.popsection");

int kernel_thread(unsigned long(* fn)(unsigned long), unsigned long arg, unsigned long flags) {
	struct pt_regs regs;
	memset(&regs, 0, sizeof(regs));

	regs.ARM_r4 = (unsigned long)arg;
	regs.ARM_r5 = (unsigned long)fn;
	regs.ARM_r6 = (unsigned long)kernel_thread_exit;
	regs.ARM_r7 = SVC_MODE | PSR_ENDSTATE | PSR_ISETSTATE;
	regs.ARM_pc = (unsigned long)kernel_thread_helper;
	regs.ARM_cpsr = regs.ARM_r7 | PSR_I_BIT;

	return do_fork(&regs, flags, 0, 0);
}

void task_init() {
	//init_mm.pgd = (pml4t_t *)Global_CR3;
	init_mm.start_code = mms.start_code;
	init_mm.end_code = mms.end_code;
	init_mm.start_data = (unsigned long)&_data;
	init_mm.end_data = mms.end_data;
	init_mm.start_rodata = (unsigned long)&_rodata;
	init_mm.end_rodata = (unsigned long)&_erodata;
	init_mm.start_brk = mms.start_brk;
	init_mm.end_brk = 0x34000000;
	init_mm.start_stack = 0;

	list_init(&init_task_union.task.list);

	kernel_thread(init, 10, CLONE_FS | CLONE_SIGNAL);

	init_task_union.task.preempt_count = 0;
	init_task_union.task.state = TASK_RUNNING;
}
