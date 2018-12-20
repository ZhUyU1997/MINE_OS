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

#include <sys/types.h>
#include <memory.h>
#include <cpu.h>
#include <lib.h>
#include <ptrace.h>
#include <ldscript.h>
#include <pgtable.h>
#include <vfs.h>
#include <waitqueue.h>
#include <mm_types.h>

// stack size 32K
#define STACK_SIZE 32768

extern long global_pid;


#define TASK_RUNNING		(1 << 0)
#define TASK_INTERRUPTIBLE	(1 << 1)
#define	TASK_UNINTERRUPTIBLE	(1 << 2)
#define	TASK_ZOMBIE		(1 << 3)
#define	TASK_STOPPED		(1 << 4)

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

	/*0x0000,0000,0000,0000 - 0x0000,7fff,ffff,ffff user*/
	/*0xffff,8000,0000,0000 - 0xffff,ffff,ffff,ffff kernel*/
	unsigned long addr_limit;

	long pid;
	long priority;
	long vrun_time;

	long exit_code;

	struct file * file_struct[TASK_FILE_MAX];

	wait_queue_T wait_childexit;
	struct task_struct *next;
	struct task_struct *parent;
};

///////struct task_struct->flags:

#define PF_KTHREAD		(1UL << 0)
#define NEED_SCHEDULE	(1UL << 1)
#define PF_VFORK		(1UL << 2)


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
		.addr_limit = 0xffffffff,	\
		.pid = 0,			\
		.priority = 2,		\
		.vrun_time = 0,		\
		.exit_code = 0,		\
		.file_struct = {0},	\
		.next = &tsk,		\
		.parent = &tsk,		\
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

   
   
   
long get_pid();
struct task_struct *get_task(long pid);

void wakeup_process(struct task_struct *tsk);
void exit_files(struct task_struct *tsk);

unsigned long do_fork(struct pt_regs * regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size);
unsigned long do_execve(struct pt_regs *regs, char *name, char *argv[], char *envp[]);
unsigned long do_exit(unsigned long exit_code);

void task_init();

extern void cpu_arm920_switch_mm(pgd_t *pgd, struct mm_struct *mm);

#define cpu_switch_mm(pgd,mm) cpu_arm920_switch_mm((pgd_t *)Virt_To_Phy(pgd),mm)

static inline void switch_mm(struct task_struct *prev, struct task_struct *next) {
	cpu_arm920_switch_mm(next->mm->pgd, next->mm);
}
extern void exit_mm(struct task_struct *tsk);
extern void ret_system_call(void);
extern void system_call(void);



extern struct task_struct *init_task[NR_CPUS];
extern union task_union init_task_union;
extern struct mm_struct init_mm;
extern struct thread_struct init_thread;

#endif
