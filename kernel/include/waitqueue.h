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

#ifndef __WAITQUEUE_H__

#define __WAITQUEUE_H__

#include "lib.h"

typedef struct {
	struct list_head wait_list;
	struct task_t *tsk;
} wait_queue_t;

void wait_queue_init(wait_queue_t * wait_queue, struct task_t *tsk);

void sleep_on(wait_queue_t * wait_queue_head);

void wakeup(wait_queue_t * wait_queue_head);

#endif
