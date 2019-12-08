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
#include <types.h>
#include <unistd.h>
#include <stdio.h>
#include <printk.h>
#include <ldscript.h>
#include <sys/err.h>
#include <assert.h>
#include <pgtable.h>
#include <mmu.h>
#include <task.h>
#include <ptrace.h>
#include <lib.h>
#include <memory.h>
#include <linkage.h>
#include <schedule.h>
#include <smp.h>
#include <sched.h>
#include <errno.h>
#include <fcntl.h>
#include <waitqueue.h>

struct mm_struct init_mm = {0};

struct thread_struct init_thread = {
	.trap_nr = 0,
	.error_code = 0
};

union task_union init_task_union = {INIT_TASK(init_task_union.task)};
struct task_t *init_task[NR_CPUS] = {&init_task_union.task};
struct task_t *current = NULL;

long global_pid;

struct file * open_exec_file(char * path) {
	struct dentry * dentry = NULL;
	struct file * filp = NULL;

	dentry = path_walk(path, 0, NULL);

	if (dentry == NULL)
		return (void *) - ENOENT;
	if (S_ISDIR(dentry->d_inode->i_mode))
		return (void *) - ENOTDIR;

	filp = (struct file *)kmalloc(sizeof(struct file), 0);
	if (filp == NULL)
		return (void *) - ENOMEM;

	filp->f_pos = 0;
	filp->flags = 0;
	filp->dentry = dentry;
	filp->flags = O_RDONLY;
	filp->f_ops = dentry->d_inode->f_ops;

	return filp;
}

unsigned long do_execve(struct pt_regs *regs, char *name, char *argv[], char *envp[]) {
	unsigned long code_start_addr = 0x800000;
	unsigned long stack_start_addr = 0xa00000;
	unsigned long brk_start_addr = 0xc00000;
	struct Page * p = NULL;
	struct file * filp = NULL;
	unsigned long retval = 0;
	long pos = 0;

	//TODO:释放旧页表
	unsigned long flags;
	raw_local_irq_save(flags);

	if (current->flags & PF_VFORK) {
		current->mm = (struct mm_struct *)kzalloc(sizeof(struct mm_struct), 0);
		current->mm->pgd = (pgd_t *)kzalloc(PAGE_16K_SIZE, 0);
		//memset(current->mm->pgd, 0, PAGE_16K_SIZE);
		//memcpy(current->mm->pgd + 384, init_task[smp_processor_id()]->mm->pgd + 384, (4096 - 768)*4); //copy kernel space

		//TODO：有部分IO地址没有映射到地址空间，暂时这样处理
		memcpy(current->mm->pgd, init_task[smp_processor_id()]->mm->pgd, PAGE_16K_SIZE);
		memset(current->mm->pgd[code_start_addr >> PAGE_2M_SHIFT], 0, ((brk_start_addr - code_start_addr) >> PAGE_1M_SHIFT)*4); //copy kernel space

	}

	assert(!(((unsigned long )current->mm->pgd) & ( PAGE_16K_SIZE - 1)));
	p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
	pgd_t *pgd = pgd_offset(current->mm, code_start_addr);
	set_pgd(pgd, p->PHY_address, MMU_FULL_ACCESS, MAP_TYPE_CB);

	switch_mm(current, current);

	filp = open_exec_file(name);
	if (IS_ERR(filp))
		return (unsigned long)filp;

	if (!(current->flags & PF_KTHREAD))
		current->addr_limit = CONFIG_TASK_SIZE;

	current->mm->start_code = code_start_addr;
	current->mm->end_code = 0;
	current->mm->start_data = 0;
	current->mm->end_data = 0;
	current->mm->start_rodata = 0;
	current->mm->end_rodata = 0;
	current->mm->start_bss = code_start_addr + filp->dentry->d_inode->i_size;
	current->mm->end_bss = stack_start_addr;
	current->mm->start_brk = brk_start_addr;
	current->mm->end_brk = brk_start_addr;
	current->mm->start_stack = stack_start_addr;

	exit_files(current);

	current->flags &= ~PF_VFORK;

	if (argv != NULL) {
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

	retval = filp->f_ops->read(filp, (void *)code_start_addr, filp->dentry->d_inode->i_size, &pos);

	print_hex(code_start_addr, 128);

	regs->ARM_pc = code_start_addr;
	regs->ARM_sp = stack_start_addr;
	regs->ARM_cpsr = SVC_MODE | PSR_ENDSTATE | PSR_ISETSTATE;
	//regs->ARM_r0 = 1;

	color_printk(RED, BLACK, "do_execve task is running");

	raw_local_irq_restore(flags);
	return retval;
}


unsigned long init(unsigned long arg) {
	color_printk(RED, BLACK, "init task is running,arg:%#018lx", arg);

	struct pt_regs *regs = (struct pt_regs *)((unsigned long)current + STACK_SIZE - sizeof(struct pt_regs));
	current->flags &= ~PF_KTHREAD;

	if(IS_ERR_VALUE(do_execve(regs, "/init.bin", NULL, NULL))){
		color_printk(RED, BLACK, "do_execve error");
		return 1;
	}

	//TODO:考虑CPSR
	asm	volatile(
		"mov	sp, %0		\n\t"
		"b		ret_system_call\n\t"
		:
		:"r"(regs)
		:"memory"
	);
	return 1;
}

extern void kernel_thread_helper(void);
asm(	".pushsection .text\n"
		"	.align\n"
		"	.type	kernel_thread_helper, #function\n"
		"kernel_thread_helper:\n"
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

inline void wakeup_process(struct task_t *tsk) {
	task_resume(tsk);
	current->flags |= NEED_SCHEDULE;
}

unsigned long copy_flags(unsigned long clone_flags, struct task_t *tsk) {
	if (clone_flags & CLONE_VM)
		tsk->flags |= PF_VFORK;
	return 0;
}

unsigned long copy_files(unsigned long clone_flags, struct task_t *tsk) {
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
void exit_files(struct task_t *tsk) {
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


unsigned long copy_mm(unsigned long clone_flags, struct task_t *tsk) {
	int error = 0;
	struct mm_struct *newmm = NULL;
	unsigned long code_start_addr = 0x800000;
	unsigned long stack_start_addr = 0xa00000;
	unsigned long brk_start_addr = 0xc00000;
	struct Page * p = NULL;

	if (clone_flags & CLONE_VM) {
		newmm = current->mm;
		goto out;
	}

	newmm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct), 0);
	memcpy(newmm, current->mm, sizeof(struct mm_struct));

	newmm->pgd = (pgd_t *)kmalloc(PAGE_16K_SIZE, 0);
	assert(!(((unsigned long)newmm->pgd) & ( PAGE_16K_SIZE - 1)));
	memcpy(newmm->pgd + 384, init_task[smp_processor_id()]->mm->pgd + 384, (4096 - 768)*4);	//copy kernel space
	memset(newmm->pgd, 0, (768)*4);	//copy user code & data & bss space

	if (current->mm->start_stack - current->mm->start_code != 0) {
		p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
		pgd_t *pgd = pgd_offset(newmm, code_start_addr);
		set_pgd(pgd, p->PHY_address, MMU_FULL_ACCESS, MAP_TYPE_CB);
		memcpy((void *)phy_to_virt(p->PHY_address), (void *)code_start_addr, stack_start_addr - code_start_addr);
	}

	////copy user brk space
	if (current->mm->end_brk - current->mm->start_brk != 0) {
		p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
		pgd_t *pgd = pgd_offset(newmm, brk_start_addr);
		set_pgd(pgd, p->PHY_address, MMU_FULL_ACCESS, MAP_TYPE_CB);
		memcpy(phy_to_virt(p->PHY_address), (void *)brk_start_addr, PAGE_2M_SIZE);
	}

out:
	tsk->mm = newmm;
	return error;
}
void exit_mm(struct task_t *tsk) {
	unsigned long code_start_addr = 0x800000;
	unsigned long * tmp;

	if (tsk->flags & PF_VFORK)
		return;

	if (tsk->mm->pgd != NULL) {
		tmp = phy_to_virt(tsk->mm->pgd[code_start_addr >> PGDIR_SHIFT][0] & ~(MMU_SECTION_SIZE - 1));
		free_pages(phy_to_2M_page(tmp), 1);
		kfree(phy_to_virt(tsk->mm->pgd));
	}
	if (tsk->mm != NULL)
		kfree(tsk->mm);
}

unsigned long copy_thread(unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size, struct task_t *tsk, struct pt_regs * regs) {
	struct pt_regs *childregs = NULL;
	struct thread_struct *thd = (struct thread_struct *)(tsk + 1);

	memset(thd, 0, sizeof(*thd));
	tsk->thread = thd;

	childregs = (struct pt_regs *)((unsigned long)tsk + STACK_SIZE) - 1;

	*childregs = *regs;

	childregs->ARM_r0 = 0;
	if (tsk->flags & PF_KTHREAD){
		childregs->ARM_sp = (unsigned long)childregs;
	}
	else
		childregs->ARM_sp = (unsigned long)stack_start;

	tsk->cpu_context.sp = (unsigned long)childregs;

	//TODO:参考linux完善
	//if (tsk->flags & PF_KTHREAD)
	//	tsk->cpu_context.pc = (unsigned long)kernel_thread_helper;
	//else

	tsk->cpu_context.pc = (unsigned long)ret_system_call;

	LOG("current  ret addr(%#08lx), sp(%#08lx)", regs->ARM_pc, regs->ARM_sp);
	LOG("new      ret addr(%#08lx), sp(%#08lx)", childregs->ARM_pc, childregs->ARM_sp);

	return 0;
}

void exit_thread(struct task_t *tsk) {

}


unsigned long do_fork(struct pt_regs *regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size) {
	int retval = 0;
	struct task_t *tsk = task_create(NULL, "", STACK_SIZE, 0);
	LOG("struct task_struct address(%#08lx)", (unsigned long)tsk);

	if (tsk == NULL) {
		retval = -EAGAIN;
		goto alloc_copy_task_fail;
	}
	
	// copy attr
	tsk->flags = current->flags;
	tsk->signal = current->signal;
	tsk->addr_limit = current->addr_limit;
	tsk->pwd = current->pwd;
	
	init_list_head(&tsk->list);
	tsk->pid = global_pid++;
	tsk->preempt_count = 0;
	tsk->cpu_id = smp_processor_id();
	list_add(&tsk->child_node, &current->child_list);
	tsk->parent = current;
	wait_queue_init(&tsk->wait_childexit, NULL);

	retval = -ENOMEM;

	if (copy_flags(clone_flags, tsk))
		goto copy_flags_fail;

	if (copy_mm(clone_flags, tsk))
		goto copy_mm_fail;

	if (copy_files(clone_flags, tsk))
		goto copy_files_fail;

	if (copy_thread(clone_flags, stack_start, stack_size, tsk, regs))
		goto copy_thread_fail;

	retval = tsk->pid;

	wakeup_process(tsk);

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
	wakeup(&current->parent->wait_childexit);
}

unsigned long do_exit(unsigned long exit_code) {
	struct task_t *tsk = current;
	LOG("exit task is running, arg(%#08lx)", exit_code);

do_exit_again:

	cli();
	tsk->exit_code = exit_code;
	exit_thread(tsk);
	exit_files(tsk);
	sti();
	
	exit_notify();
	task_zombie(current);
	
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

	return do_fork(&regs, flags | CLONE_VM, 0, 0);
}

void task_init() {
	local_irq_disable();
	kernel_thread(init, 10, CLONE_FS | CLONE_SIGHAND);
	void microui(void);
	kernel_thread(microui, 10, CLONE_FS | CLONE_SIGHAND);
	int cmd_loop();
	kernel_thread(cmd_loop, 10, CLONE_FS | CLONE_SIGHAND);
	local_irq_enable();
}

