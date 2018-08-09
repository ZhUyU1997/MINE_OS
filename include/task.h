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

#ifndef __TASK_H__
#define __TASK_H__

#include "sys/types.h"
#include "memory.h"
#include "cpu.h"
#include "lib.h"
#include "ptrace.h"
#include "ldscript.h"
#include "pgtable.h"

#define CLONE_FS	(1 << 0)
#define CLONE_FILES	(1 << 1)
#define CLONE_SIGNAL	(1 << 2)


// stack size 32K
#define STACK_SIZE 32768




#define TASK_RUNNING		(1 << 0)
#define TASK_INTERRUPTIBLE	(1 << 1)
#define	TASK_UNINTERRUPTIBLE	(1 << 2)
#define	TASK_ZOMBIE		(1 << 3)
#define	TASK_STOPPED		(1 << 4)

struct mm_struct {
	pgd_t *pgd;	//page table point

	unsigned long start_code, end_code;
	unsigned long start_data, end_data;
	unsigned long start_rodata, end_rodata;
	unsigned long start_bss, end_bss;
	unsigned long start_brk, end_brk;
	unsigned long start_stack;
};

struct thread_struct {
	unsigned long address;
	unsigned long trap_nr;
	unsigned long error_code;
};

struct cpu_context_save {
	__u32	r4;
	__u32	r5;
	__u32	r6;
	__u32	r7;
	__u32	r8;
	__u32	r9;
	__u32	sl;
	__u32	fp;
	__u32	sp;
	__u32	pc;
	__u32	extra[2];		/* Xscale 'acc' register, etc */
};

/*

*/

#define TASK_FILE_MAX	10

struct task_struct {
	volatile long state;
	unsigned long flags;
	long preempt_count;
	long signal;
	long cpu_id;		//CPU ID

	struct mm_struct *mm;
	struct thread_struct *thread;
	struct cpu_context_save cpu_context;
	struct List list;

	unsigned long addr_limit;	/*0x0000,0000,0000,0000 - 0x0000,7fff,ffff,ffff user*/
	/*0xffff,8000,0000,0000 - 0xffff,ffff,ffff,ffff kernel*/
	long pid;
	long priority;
	long vrun_time;
};

///////struct task_struct->flags:

#define PF_KTHREAD	(1UL << 0)
#define NEED_SCHEDULE	(1UL << 1)


union task_union {
	struct task_struct task;
	unsigned long stack[STACK_SIZE / sizeof(unsigned long)];
} __attribute__((aligned(8)));	//8Bytes align


extern struct thread_struct init_thread;

#define INIT_TASK(tsk)	\
	{			\
		.state = TASK_UNINTERRUPTIBLE,		\
		.flags = PF_KTHREAD,		\
		.preempt_count = 0,		\
		.signal = 0,		\
		.cpu_id = 0,		\
		.mm = &init_mm,			\
		.thread = &init_thread,		\
		.addr_limit = 0xc0000000,	\
		.pid = 0,			\
		.priority = 2,		\
		.vrun_time = 0,		\
	}


extern struct task_struct * current;
static inline struct task_struct * get_current() {
	return current;
}

#define GET_CURRENT get_current()

/*

*/

extern void __switch_to(struct cpu_context_save *, struct cpu_context_save *);

#define switch_to(prev,next)			\
	do {									\
		struct task_struct *temp = prev;	\
		current = next;						\
		__switch_to(&temp->cpu_context, &next->cpu_context);	\
	} while (0)

unsigned long do_fork(struct pt_regs * regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size);
void task_init();

#define MAX_SYSTEM_CALL_NR 128

typedef unsigned long (* system_call_t)(struct pt_regs * regs);

unsigned long no_system_call(struct pt_regs * regs);

unsigned long sys_printf(struct pt_regs * regs);

extern void ret_system_call(void);
extern void system_call(void);

extern system_call_t system_call_table[MAX_SYSTEM_CALL_NR];


extern struct task_struct *init_task[NR_CPUS];
extern union task_union init_task_union;
extern struct mm_struct init_mm;
extern struct thread_struct init_thread;

#endif
