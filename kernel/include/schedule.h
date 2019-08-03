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

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include "task.h"

struct scheduler_t {
	struct rb_root_cached ready;
	struct list_head suspend;
	struct task_t * running;
	uint64_t min_vtime;
	uint64_t weight;
	spinlock_t lock;
};

extern struct scheduler_t __sched[NR_CPUS];

void schedule();
void do_init_sched();

struct task_t * task_create(struct scheduler_t * sched, const char * name, size_t stksz, int nice);
void task_destroy(struct task_t * task);
void task_zombie(struct task_t * task);
void task_renice(struct task_t * task, int nice);
void task_suspend(struct task_t * task);
void task_resume(struct task_t * task);

#endif
