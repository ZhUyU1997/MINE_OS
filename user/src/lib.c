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

#include "syscall.h"


#define SYSFUNC_DEF(name)	_SYSFUNC_DEF_(name,__NR_##name)
#define _SYSFUNC_DEF_(name,nr)	__SYSFUNC_DEF__(name,nr)
#define __SYSFUNC_DEF__(name,nr)	\
	extern int name ()
	__asm__	(		\
		".global "#name"	\n\t"	\
		".type	"#name",	@function \n\t"	\
		#name":		\n\t"	\
		"b   syscall	\n\t"	\
	);


SYSFUNC_DEF(putstring)
SYSFUNC_DEF(open)
SYSFUNC_DEF(close)
SYSFUNC_DEF(read)
SYSFUNC_DEF(write)
SYSFUNC_DEF(lseek)
SYSFUNC_DEF(fork)
SYSFUNC_DEF(vfork)
SYSFUNC_DEF(execve)
SYSFUNC_DEF(exit)
SYSFUNC_DEF(wait4)
SYSFUNC_DEF(brk)
SYSFUNC_DEF(reboot)
SYSFUNC_DEF(chdir)
SYSFUNC_DEF(getdents)





