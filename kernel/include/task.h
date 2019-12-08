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

#include <types.h>
#include <memory.h>
#include <cpu.h>
#include <lib.h>
#include <ptrace.h>
#include <ldscript.h>
#include <pgtable.h>
#include <vfs.h>
#include <waitqueue.h>
#include <mm_types.h>
#include <rbtree_augmented.h>

// stack size 32K
#define STACK_SIZE 32768

extern long global_pid;


#define TASK_STATUS_RUNNING	 1
#define TASK_STATUS_SUSPEND	 2
#define TASK_STATUS_ZOMBIE	 4

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
	__u32	extra[16];
};

/*

*/

#define TASK_FILE_MAX	10

struct task_t {
	volatile long status;
	unsigned long flags;
	long preempt_count;
	long signal;
	long cpu_id;		//CPU ID

	struct mm_struct *mm;
	struct thread_struct *thread;
	struct cpu_context_save cpu_context;
	struct list_head list;

	unsigned long addr_limit;

	long pid;

	uint64_t start;
	uint64_t time;
	uint64_t vtime;
	char * name;
	
	int nice;
	int weight;
	uint32_t inv_weight;

	long exit_code;

	struct rb_node node;
	struct scheduler_t * sched;

	struct file * file_struct[TASK_FILE_MAX];
	struct dentry *pwd;
	wait_queue_t wait_childexit;
	struct list_head child_node;
	struct list_head child_list;
	struct task_t *parent;
};

///////struct task_struct->flags:

#define PF_KTHREAD		(1UL << 0)
#define NEED_SCHEDULE	(1UL << 1)
#define PF_VFORK		(1UL << 2)


union task_union {
	struct task_t task;
	unsigned long stack[STACK_SIZE / sizeof(unsigned long)];
} __attribute__((aligned(8)));	//8Bytes align


extern struct thread_struct init_thread;

#define INIT_TASK(tsk)	\
	{			\
		.status = TASK_STATUS_SUSPEND,		\
		.flags = PF_KTHREAD,		\
		.preempt_count = 0,		\
		.signal = 0,		\
		.cpu_id = 0,		\
		.mm = &init_mm,			\
		.thread = &init_thread,		\
		.addr_limit = 0xffffffff,	\
		.pid = 0,			\
		.nice = 0,		\
		.vtime = 0,		\
		.exit_code = 0,		\
		.file_struct = {0},	\
		.parent = &tsk,		\
	}

extern struct task_t * current;
static inline struct task_t * get_current() {
	return current;
}

#define GET_CURRENT get_current()

/*

*/

extern void __switch_to(struct cpu_context_save *, struct cpu_context_save *);

#define switch_to(prev,next)			\
	do {									\
		struct task_t *temp = prev;	\
		current = next;						\
		__switch_to(&temp->cpu_context, &next->cpu_context + 1);	\
	} while (0)

long get_pid();
int kernel_thread(unsigned long(* fn)(unsigned long), unsigned long arg, unsigned long flags);
void wakeup_process(struct task_t *tsk);
void exit_files(struct task_t *tsk);

unsigned long do_fork(struct pt_regs * regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size);
unsigned long do_execve(struct pt_regs *regs, char *name, char *argv[], char *envp[]);
unsigned long do_exit(unsigned long exit_code);

void task_init();

extern void cpu_v7_switch_mm(pgd_t *pgd, struct mm_struct *mm);

#define cpu_switch_mm(pgd,mm) cpu_v7_switch_mm((pgd_t *)virt_to_phy(pgd),mm)

static inline void switch_mm(struct task_t *prev, struct task_t *next) {
	cpu_switch_mm(next->mm->pgd, next->mm);
}
extern void exit_mm(struct task_t *tsk);
extern void ret_system_call(void);
extern void system_call(void);

extern struct task_t *init_task[NR_CPUS];
extern union task_union init_task_union;
extern struct mm_struct init_mm;
extern struct thread_struct init_thread;

#endif
