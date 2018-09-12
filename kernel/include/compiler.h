#ifndef __COMPILER_H__
#define __COMPILER_H__

#define __acquire(x) (void)0
#define __release(x) (void)0

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

/* Optimization barrier */
#ifndef barrier
# define barrier() __memory_barrier()
#endif

/* Unreachable code */
#ifndef unreachable
# define unreachable() do { } while (1)
#endif

#endif
