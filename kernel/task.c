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
#include "global_config.h"
#include "assert.h"
#include "pgtable.h"
#include "mmu.h"
#include "timer.h"
#include "task.h"
#include "ptrace.h"
#include "lib.h"
#include "memory.h"
#include "linkage.h"
#include "schedule.h"
#include "printk.h"
#include "smp.h"
#include "unistd.h"
#include "stdio.h"
#include "sched.h"
#include "sched.h"
#include "errno.h"
#include "fcntl.h"
#include "waitqueue.h"

struct mm_struct init_mm = {0};

struct thread_struct init_thread = {
	.trap_nr = 0,
	.error_code = 0
};

union task_union init_task_union = {INIT_TASK(init_task_union.task)};
struct task_struct *init_task[NR_CPUS] = {&init_task_union.task,0};
struct task_struct *current = NULL;

long global_pid;

struct task_struct *get_task(long pid) {
	struct task_struct *tsk = NULL;
	for (tsk = init_task_union.task.next; tsk != &init_task_union.task; tsk = tsk->next) {
		if (tsk->pid == pid)
			return tsk;
	}
	return NULL;
}

struct file * open_exec_file(char * path) {
	struct dir_entry * dentry = NULL;
	struct file * filp = NULL;

	dentry = path_walk(path, 0);

	if (dentry == NULL)
		return (void *) - ENOENT;
	if (dentry->dir_inode->attribute == FS_ATTR_DIR)
		return (void *) - ENOTDIR;

	filp = (struct file *)kmalloc(sizeof(struct file), 0);
	if (filp == NULL)
		return (void *) - ENOMEM;

	filp->position = 0;
	filp->mode = 0;
	filp->dentry = dentry;
	filp->mode = O_RDONLY;
	filp->f_ops = dentry->dir_inode->f_ops;

	return filp;
}

unsigned long do_execve(struct pt_regs *regs, char *name, char *argv[], char *envp[]) {
	unsigned long code_start_addr = 0x800000;
	unsigned long stack_start_addr = 0xa00000;
	unsigned long brk_start_addr = 0xc00000;
	unsigned long * tmp;
	unsigned long * virtual = NULL;
	struct Page * p = NULL;
	struct file * filp = NULL;
	unsigned long retval = 0;
	long pos = 0;

	if (current->flags & PF_VFORK) {
		current->mm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct), 0);
		memset(current->mm, 0, sizeof(struct mm_struct));

		current->mm->pgd = (pgd_t *)kmalloc(PAGE_16K_SIZE, 0);
		color_printk(RED, BLACK, "load_binary_file malloc new pgd:%#018lx\n", current->mm->pgd);
		memset(current->mm->pgd, 0, PAGE_16K_SIZE);
		memcpy(current->mm->pgd + 384, init_task[SMP_cpu_id()]->mm->pgd + 384, (4096 - 768)*4);	//copy kernel space
	}

	pgd_t *pgd = current->mm->pgd;
	assert(!(((unsigned long )pgd) & ( PAGE_16K_SIZE - 1)));
	p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
	set_pgd(pgd, 0x800000, Virt_To_Phy(p->PHY_address), 1, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);

	//__asm__ __volatile__("movq	%0,	%%cr3	\n\t"::"r"(current->mm->pgd):"memory");

	filp = open_exec_file(name);
	if ((unsigned long)filp > -0x1000UL)
		return (unsigned long)filp;

	if (!(current->flags & PF_KTHREAD))
		current->addr_limit = TASK_SIZE;

	current->mm->start_code = code_start_addr;
	current->mm->end_code = 0;
	current->mm->start_data = 0;
	current->mm->end_data = 0;
	current->mm->start_rodata = 0;
	current->mm->end_rodata = 0;
	current->mm->start_bss = code_start_addr + filp->dentry->dir_inode->file_size;
	current->mm->end_bss = stack_start_addr;
	current->mm->start_brk = brk_start_addr;
	current->mm->end_brk = brk_start_addr;
	current->mm->start_stack = stack_start_addr;

	exit_files(current);

	current->flags &= ~PF_VFORK;

	if (argv != NULL) {
		int argc = 0;
		int len = 0;
		int i = 0;
		char ** dargv = (char **)(stack_start_addr - 10 * sizeof(char *));
		pos = (unsigned long)dargv;

		for (i = 0; i < 10 && argv[i] != NULL; i++) {
			len = strnlen_user(argv[i], 1024) + 1;
			strcpy((char *)(pos - len), argv[i]);
			dargv[i] = (char *)(pos - len);
			pos -= len;
		}
		stack_start_addr = pos - 10;
		regs->ARM_r0 = i;	//argc
		regs->ARM_r1 = (unsigned long)dargv;	//argv
	}

	memset((void *)code_start_addr, 0, stack_start_addr - code_start_addr);

	pos = 0;
	retval = filp->f_ops->read(filp, (void *)code_start_addr, filp->dentry->dir_inode->file_size, &pos);

	regs->ARM_pc = code_start_addr;
	regs->ARM_sp = stack_start_addr;
	//regs->ARM_r0 = 1;

	color_printk(RED, BLACK, "do_execve task is running\n");

	return retval;
}

unsigned long init(unsigned long arg) {
	DISK1_FAT32_FS_init();

	color_printk(RED, BLACK, "init task is running,arg:%#018lx\n", arg);

	current->cpu_context.pc = (unsigned long)ret_system_call;
	current->cpu_context.sp = (unsigned long)current + STACK_SIZE - sizeof(struct pt_regs);
	current->flags &= ~PF_KTHREAD;

	do_execve(current->cpu_context.sp, "/init.bin", NULL, NULL);

	//TODO:考虑CPSR
	__asm__	__volatile__("mov	sp, %0		\n\t"
						 "ldmia	sp, {r0 - pc}			@ load r0 - pc\n\t"
						 :
						 :"r"(current->cpu_context.sp)
						 :"memory"
						);
	return 1;
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

inline void wakeup_process(struct task_struct *tsk) {
	tsk->state = TASK_RUNNING;
	insert_task_queue(tsk);
	current->flags |= NEED_SCHEDULE;
}

unsigned long copy_flags(unsigned long clone_flags, struct task_struct *tsk) {
	if (clone_flags & CLONE_VM)
		tsk->flags |= PF_VFORK;
	return 0;
}

unsigned long copy_files(unsigned long clone_flags, struct task_struct *tsk) {
	int error = 0;
	int i = 0;
	if (clone_flags & CLONE_FS)
		goto out;

	for (i = 0; i < TASK_FILE_MAX; i++)
		if (current->file_struct[i] != NULL) {
			tsk->file_struct[i] = (struct file *)kmalloc(sizeof(struct file), 0);
			memcpy(tsk->file_struct[i], current->file_struct[i], sizeof(struct file));
		}
out:
	return error;
}
void exit_files(struct task_struct *tsk) {
	int i = 0;
	if (tsk->flags & PF_VFORK)
		;
	else
		for (i = 0; i < TASK_FILE_MAX; i++)
			if (tsk->file_struct[i] != NULL) {
				kfree(tsk->file_struct[i]);
			}

	memset(tsk->file_struct, 0, sizeof(struct file *) * TASK_FILE_MAX);	//clear current->file_struct
}


unsigned long copy_mm(unsigned long clone_flags, struct task_struct *tsk) {
	int error = 0;
	struct mm_struct *newmm = NULL;
	unsigned long code_start_addr = 0x800000;
	unsigned long stack_start_addr = 0xa00000;
	unsigned long brk_start_addr = 0xc00000;
	unsigned long * tmp;
	unsigned long * virtual = NULL;
	struct Page * p = NULL;

	if (clone_flags & CLONE_VM) {
		newmm = current->mm;
		goto out;
	}

	newmm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct), 0);
	memcpy(newmm, current->mm, sizeof(struct mm_struct));

	newmm->pgd = (pgd_t *)kmalloc(PAGE_16K_SIZE, 0);
	memcpy(newmm->pgd + 384, init_task[SMP_cpu_id()]->mm->pgd + 384, (4096 - 768)*4);	//copy kernel space
	memset(newmm->pgd, 0, (768)*4);	//copy user code & data & bss space

	pgd_t *pgd = newmm->pgd;
	assert(!(((unsigned long)pgd) & ( PAGE_16K_SIZE - 1)));
	p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
	set_pgd(pgd, 0x800000, Virt_To_Phy(p->PHY_address), 1, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);

	memcpy(Phy_To_Virt(p->PHY_address), code_start_addr, stack_start_addr - code_start_addr);

	////copy user brk space
	if (current->mm->end_brk - current->mm->start_brk != 0) {
		p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
		set_pgd(pgd, brk_start_addr, Virt_To_Phy(p->PHY_address), 1, MMU_FULL_ACCESS, MMU_DOMAIN(0), MMU_CACHE_ENABLE, MMU_BUFFER_ENABLE);
		memcpy(Phy_To_Virt(p->PHY_address), (void *)brk_start_addr, PAGE_2M_SIZE);
	}

out:
	tsk->mm = newmm;
	return error;
}
void exit_mm(struct task_struct *tsk) {
	unsigned long code_start_addr = 0x800000;
	unsigned long * tmp;

	if (tsk->flags & PF_VFORK)
		return;

	if (tsk->mm->pgd != NULL) {
		tmp = Phy_To_Virt(tsk->mm->pgd[code_start_addr >> PGDIR_SHIFT][0] & (MMU_SECTION_SIZE - 1));
		free_pages(Phy_to_2M_Page(tmp), 1);
		kfree(Phy_To_Virt(tsk->mm->pgd));
	}
	if (tsk->mm != NULL)
		kfree(tsk->mm);
}

unsigned long copy_thread(unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size, struct task_struct *tsk, struct pt_regs * regs) {
	struct pt_regs *childregs = NULL;
	struct thread_struct *thd = (struct thread_struct *)(tsk + 1);

	memset(thd, 0, sizeof(*thd));
	tsk->thread = thd;

	childregs = (struct pt_regs *)((unsigned long)tsk + STACK_SIZE) - 1;

	memcpy(childregs, regs, sizeof(struct pt_regs));
	childregs->ARM_r0 = 0;
	childregs->ARM_sp = stack_start;

	tsk->cpu_context.sp = (unsigned long)childregs;

	if (tsk->flags & PF_KTHREAD)
		tsk->cpu_context.pc = (unsigned long)kernel_thread_helper;
	else
		tsk->cpu_context.pc = (unsigned long)ret_system_call;

	color_printk(WHITE, BLACK, "current user ret addr:%#018lx,sp:%#018lx\n", regs->ARM_pc, regs->ARM_sp);
	color_printk(WHITE, BLACK, "new user ret addr:%#018lx,sp:%#018lx\n", childregs->ARM_pc, childregs->ARM_sp);

	return 0;
}

void exit_thread(struct task_struct *tsk) {

}


unsigned long do_fork(struct pt_regs *regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size) {
	int retval = 0;
	struct task_struct *tsk = NULL;

	tsk = (struct task_struct *)kmalloc(STACK_SIZE, 0);
	color_printk(WHITE, BLACK, "struct task_struct address:%#018lx\n", (unsigned long)tsk);

	if (tsk == NULL) {
		retval = -EAGAIN;
		goto alloc_copy_task_fail;
	}

	memset(tsk, 0, sizeof(struct task_struct));

	*tsk = *current;

	list_init(&tsk->list);
	tsk->priority = 2;
	tsk->pid = global_pid++;
	tsk->preempt_count = 0;
	tsk->cpu_id = SMP_cpu_id();
	tsk->state = TASK_UNINTERRUPTIBLE;
	tsk->next = init_task_union.task.next;
	init_task_union.task.next = tsk;
	tsk->parent = current;
	wait_queue_init(&tsk->wait_childexit, NULL);

	retval = -ENOMEM;
//	copy flags
	if (copy_flags(clone_flags, tsk))
		goto copy_flags_fail;

//	copy mm struct
	if (copy_mm(clone_flags, tsk))
		goto copy_mm_fail;

//	copy file struct
	if (copy_files(clone_flags, tsk))
		goto copy_files_fail;

//	copy thread struct
	if (copy_thread(clone_flags, stack_start, stack_size, tsk, regs))
		goto copy_thread_fail;

	retval = tsk->pid;

	wakeup_process(tsk);

fork_ok:
	return retval;


copy_thread_fail:
	exit_thread(tsk);
copy_files_fail:
	exit_files(tsk);
copy_mm_fail:
	exit_mm(tsk);
copy_flags_fail:
alloc_copy_task_fail:
	kfree(tsk);

	return retval;
}


void exit_notify(void) {
	wakeup(&current->parent->wait_childexit, TASK_INTERRUPTIBLE);
}


unsigned long do_exit(unsigned long exit_code) {
	struct task_struct *tsk = current;
	color_printk(RED, BLACK, "exit task is running,arg:%#018lx\n", exit_code);

do_exit_again:

	cli();
	tsk->state = TASK_ZOMBIE;
	tsk->exit_code = exit_code;
	exit_thread(tsk);
	exit_files(tsk);
	sti();

	exit_notify();
	schedule();

	goto do_exit_again;
	return 0;
}

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
	init_mm.pgd = (pgd_t *)MUM_TLB_BASE_ADDR;
	init_mm.start_code = mms.start_code;
	init_mm.end_code = mms.end_code;
	init_mm.start_data = (unsigned long)&_data;
	init_mm.end_data = mms.end_data;
	init_mm.start_rodata = (unsigned long)&_rodata;
	init_mm.end_rodata = (unsigned long)&_erodata;
	init_mm.start_bss = (unsigned long)&_bss;
	init_mm.end_bss = (unsigned long)&_ebss;
	init_mm.start_brk = mms.start_brk;
	init_mm.end_brk = current->addr_limit;
	init_mm.start_stack = 0;

	list_init(&init_task_union.task.list);

	kernel_thread(init, 10, CLONE_FS | CLONE_SIGNAL);

	init_task_union.task.preempt_count = 0;
	init_task_union.task.state = TASK_RUNNING;
}

