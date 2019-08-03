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

#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include "irqflags.h"

typedef struct {
	union {
		__volatile__ long value;
		__volatile__ long counter;
	};
} atomic_T, atomic_t;

#define atomic_read(atomic)	((atomic)->value)
#define atomic_set(atomic,val)	(((atomic)->value) = (val))

static inline int atomic_add_return(int i, atomic_t *v)
{
	irq_flags_t flags;
	int val;

	raw_local_irq_save(flags);
	val = v->counter;
	v->counter = val += i;
	raw_local_irq_restore(flags);

	return val;
}

static inline void atomic_add(atomic_T * atomic, long value) {
	atomic_add_return(value, atomic);
}

static inline int atomic_sub_return(int i, atomic_t *v)
{
	irq_flags_t flags;
	int val;

	raw_local_irq_save(flags);
	val = v->counter;
	v->counter = val -= i;
	raw_local_irq_restore(flags);

	return val;
}

static inline void atomic_sub(atomic_T *atomic, long value) {
	atomic_sub_return(value, atomic);
}

static inline void atomic_inc(atomic_T *atomic) {
	atomic_add_return(1, atomic);
}

static inline void atomic_dec(atomic_T *atomic) {
	atomic_sub_return(1, atomic);
}

static inline void atomic_set_mask(atomic_T *atomic, long mask) {
	irq_flags_t flags;

	raw_local_irq_save(flags);
	atomic->value |= mask;
	raw_local_irq_restore(flags);
}

static inline void atomic_clear_mask(atomic_T *atomic, long mask) {
	irq_flags_t flags;

	raw_local_irq_save(flags);
	atomic->value &= ~mask;
	raw_local_irq_restore(flags);
}

#endif
