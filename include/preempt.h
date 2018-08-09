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

#ifndef __PREEMPT_H__

#define __PREEMPT_H__

#include "task.h"
#include "compiler.h"

#define preempt_count()	(current->preempt_count)

#define add_preempt_count(val)	do { preempt_count() += (val); } while (0)
#define sub_preempt_count(val)	do { preempt_count() -= (val); } while (0)

#define inc_preempt_count() add_preempt_count(1)
#define dec_preempt_count() sub_preempt_count(1)


#define preempt_disable() \
	do { \
		inc_preempt_count(); \
		barrier(); \
	} while (0)
#define sched_preempt_enable_no_resched() \
	do { \
		barrier(); \
		dec_preempt_count(); \
	} while (0)

#define preempt_enable_no_resched()	sched_preempt_enable_no_resched()

#define preempt_enable() \
	do { \
		preempt_enable_no_resched(); \
		barrier(); \
	} while (0)
#endif
