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

#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <types.h>
#include <typecheck.h>
#include <compiler.h>
#include <preempt.h>
#include <irqflags.h>

typedef struct {
	union {
		u32 slock;
		struct __raw_tickets {
			u16 owner;
			u16 next;
		} tickets;
	};
} arch_spinlock_t;

typedef struct raw_spinlock {
	arch_spinlock_t raw_lock;
} raw_spinlock_t;

typedef struct spinlock {
	union {
		volatile struct raw_spinlock rlock;
	};
} spinlock_T, spinlock_t;

#define __LOCK(lock)						do { preempt_disable(); __acquire(lock); (void)(lock); } while (0)
#define __LOCK_IRQ(lock)					do { local_irq_disable(); __LOCK(lock); } while (0)
#define __LOCK_IRQSAVE(lock, flags)			do { local_irq_save(flags); __LOCK(lock); } while (0)
#define __UNLOCK(lock)						do { preempt_enable(); __release(lock); (void)(lock); } while (0)
#define __UNLOCK_IRQ(lock)					do { local_irq_enable(); __UNLOCK(lock); } while (0)
#define __UNLOCK_IRQRESTORE(lock, flags)	do { local_irq_restore(flags); __UNLOCK(lock); } while (0)

#define _raw_spin_lock(lock)						__LOCK(lock)
#define _raw_spin_unlock(lock)						__UNLOCK(lock)
#define _raw_spin_lock_irq(lock)					__LOCK_IRQ(lock)
#define _raw_spin_unlock_irq(lock)					__UNLOCK_IRQ(lock)
#define _raw_spin_lock_irqsave(lock, flags)			__LOCK_IRQSAVE(lock, flags)
#define _raw_spin_unlock_irqrestore(lock, flags)	__UNLOCK_IRQRESTORE(lock, flags)
#define _raw_spin_trylock(lock)						({ __LOCK(lock); 1; })

#define raw_spin_lock_init(lock) do { *(lock) = (struct raw_spinlock){ 0 }; } while (0)

#define raw_spin_lock(lock)	_raw_spin_lock(lock)
#define raw_spin_unlock(lock)		_raw_spin_unlock(lock)
#define raw_spin_trylock(lock)	_raw_spin_trylock(lock)
#define raw_spin_lock_irq(lock)		_raw_spin_lock_irq(lock)
#define raw_spin_unlock_irq(lock)	_raw_spin_unlock_irq(lock)
#define raw_spin_lock_irqsave(lock, flags)		\
	do {						\
		typecheck(irq_flags_t, flags);	\
		_raw_spin_lock_irqsave(lock, flags);	\
	} while (0)
#define raw_spin_unlock_irqrestore(lock, flags)		\
	do {							\
		typecheck(irq_flags_t, flags);		\
		_raw_spin_unlock_irqrestore(lock, flags);	\
	} while (0)


/*
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
}

#define SPIN_LOCK_INIT()	{ 0 }

#define spin_lock_init(_lock)				\
do {							\
	raw_spin_lock_init(spinlock_check(_lock));		\
} while (0)

static inline void spin_init(spinlock_t *lock)
{
	raw_spin_lock_init(&lock->rlock);
}

static inline void spin_lock(spinlock_t *lock)
{
	raw_spin_lock(&lock->rlock);
}

static inline void spin_unlock(spinlock_t *lock)
{
	raw_spin_unlock(&lock->rlock);
}

static inline int spin_trylock(spinlock_t *lock)
{
	return raw_spin_trylock(&lock->rlock);
}

#define spin_lock_irqsave(lock, flags)				\
do {								\
	raw_spin_lock_irqsave(spinlock_check(lock), flags);	\
} while (0)

static inline void spin_unlock_irqrestore(spinlock_t *lock, irq_flags_t flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
}

static inline void spin_lock_irq(spinlock_t *lock)
{
	raw_spin_lock_irq(&lock->rlock);
}

static inline void spin_unlock_irq(spinlock_t *lock)
{
	raw_spin_unlock_irq(&lock->rlock);
}
#endif
