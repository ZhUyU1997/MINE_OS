#ifndef __SYSCALL_H__
#define __SYSCALL_H__
#define __SC_DECL1(t1, a1)	t1 a1
#define __SC_DECL2(t2, a2, ...) t2 a2, __SC_DECL1(__VA_ARGS__)
#define __SC_DECL3(t3, a3, ...) t3 a3, __SC_DECL2(__VA_ARGS__)
#define __SC_DECL4(t4, a4, ...) t4 a4, __SC_DECL3(__VA_ARGS__)
#define __SC_DECL5(t5, a5, ...) t5 a5, __SC_DECL4(__VA_ARGS__)
#define __SC_DECL6(t6, a6, ...) t6 a6, __SC_DECL5(__VA_ARGS__)

#define __SC_LONG1(t1, a1) 	long a1
#define __SC_LONG2(t2, a2, ...) long a2, __SC_LONG1(__VA_ARGS__)
#define __SC_LONG3(t3, a3, ...) long a3, __SC_LONG2(__VA_ARGS__)
#define __SC_LONG4(t4, a4, ...) long a4, __SC_LONG3(__VA_ARGS__)
#define __SC_LONG5(t5, a5, ...) long a5, __SC_LONG4(__VA_ARGS__)
#define __SC_LONG6(t6, a6, ...) long a6, __SC_LONG5(__VA_ARGS__)

#define __SC_CAST1(t1, a1)	(t1) a1
#define __SC_CAST2(t2, a2, ...) (t2) a2, __SC_CAST1(__VA_ARGS__)
#define __SC_CAST3(t3, a3, ...) (t3) a3, __SC_CAST2(__VA_ARGS__)
#define __SC_CAST4(t4, a4, ...) (t4) a4, __SC_CAST3(__VA_ARGS__)
#define __SC_CAST5(t5, a5, ...) (t5) a5, __SC_CAST4(__VA_ARGS__)
#define __SC_CAST6(t6, a6, ...) (t6) a6, __SC_CAST5(__VA_ARGS__)

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)])) 
#define __SC_TEST(type)		BUILD_BUG_ON(sizeof(type) > sizeof(long))
#define __SC_TEST1(t1, a1)	__SC_TEST(t1)
#define __SC_TEST2(t2, a2, ...)	__SC_TEST(t2); __SC_TEST1(__VA_ARGS__)
#define __SC_TEST3(t3, a3, ...)	__SC_TEST(t3); __SC_TEST2(__VA_ARGS__)
#define __SC_TEST4(t4, a4, ...)	__SC_TEST(t4); __SC_TEST3(__VA_ARGS__)
#define __SC_TEST5(t5, a5, ...)	__SC_TEST(t5); __SC_TEST4(__VA_ARGS__)
#define __SC_TEST6(t6, a6, ...)	__SC_TEST(t6); __SC_TEST5(__VA_ARGS__)

#define SYSCALL_DEFINE1(name, ...) SYSCALL_DEFINEx(1, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE2(name, ...) SYSCALL_DEFINEx(2, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE3(name, ...) SYSCALL_DEFINEx(3, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE4(name, ...) SYSCALL_DEFINEx(4, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE5(name, ...) SYSCALL_DEFINEx(5, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE6(name, ...) SYSCALL_DEFINEx(6, _##name, __VA_ARGS__)

//TODO
#define asmlinkage
#ifdef CONFIG_HAVE_SYSCALL_WRAPPERS

#define SYSCALL_DEFINE(name) static inline long SYSC_##name(void)
//TODO

#define __stringify(x) #x
#define SYSCALL_DEFINEx(x, name, ...)					\
	asmlinkage long sys##name(__SC_DECL##x(__VA_ARGS__)) __attribute__((alias(__stringify(SyS##name))));		\
	static inline long SYSC##name(__SC_DECL##x(__VA_ARGS__));	\
	asmlinkage long SyS##name(__SC_LONG##x(__VA_ARGS__))		\
	{								\
		__SC_TEST##x(__VA_ARGS__);				\
		return (long) SYSC##name(__SC_CAST##x(__VA_ARGS__));	\
	}								\
	static inline long SYSC##name(__SC_DECL##x(__VA_ARGS__))

#else /* CONFIG_HAVE_SYSCALL_WRAPPERS */

#define SYSCALL_DEFINE(name) asmlinkage long sys_##name(void)
#define SYSCALL_DEFINEx(x, name, ...)					\
	asmlinkage long sys##name(__SC_DECL##x(__VA_ARGS__))

#endif /* CONFIG_HAVE_SYSCALL_WRAPPERS */

extern int syscall(int number,...);
#endif