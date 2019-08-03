#ifndef __PREEMPT_H__
#define __PREEMPT_H__

#include <compiler.h>

#if 1
extern void add_preempt_count(int val);
extern void sub_preempt_count(int val);
extern long preempt_count();

#else

#define preempt_count()	(current->preempt_count)

#define add_preempt_count(val)	do { preempt_count() += (val); } while (0)
#define sub_preempt_count(val)	do { preempt_count() -= (val); } while (0)

#endif
#define inc_preempt_count() add_preempt_count(1)
#define dec_preempt_count() sub_preempt_count(1)

//TODO:完善
//#ifdef CONFIG_PREEMPT_COUNT
#if 1

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

#else

#define preempt_disable()		do { } while (0)
#define sched_preempt_enable_no_resched()	do { } while (0)
#define preempt_enable_no_resched()	do { } while (0)
#define preempt_enable()		do { } while (0)

#endif
#endif
