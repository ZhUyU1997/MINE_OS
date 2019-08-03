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

#include <atomic.h>
#include <lib.h>
#include <task.h>
#include <schedule.h>
#include <waitqueue.h>
#include <semaphore.h>
#include <list.h>

void semaphore_init(semaphore_T * semaphore, unsigned long count) {
	atomic_set(&semaphore->counter, count);
	wait_queue_init(&semaphore->wait, NULL);
}

void __up(semaphore_T * semaphore) {
	wait_queue_t * wait = list_entry(&semaphore->wait.wait_list, wait_queue_t, wait_list);

	list_del(&wait->wait_list);
	task_resume(wait->tsk);
	//TODO：调度策略
	current->flags |= NEED_SCHEDULE;
}

void semaphore_up(semaphore_T * semaphore) {
	if (list_empty(&semaphore->wait.wait_list))
		atomic_inc(&semaphore->counter);
	else
		__up(semaphore);
}

void __down(semaphore_T * semaphore) {
	wait_queue_t wait;
	wait_queue_init(&wait, current);
	list_add_tail(&semaphore->wait.wait_list, &wait.wait_list);
	task_suspend(current);
	schedule();
}

void semaphore_down(semaphore_T * semaphore) {
	if (atomic_read(&semaphore->counter) > 0)
		atomic_dec(&semaphore->counter);
	else
		__down(semaphore);
}

