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

#ifndef __SYSCALL_H__

#define __SYSCALL_H__

#define	__NR_putstring	1
#define	__NR_open	2
#define	__NR_close	3
#define	__NR_read	4
#define	__NR_write	5
#define	__NR_lseek	6
#define	__NR_fork	7
#define	__NR_vfork	8
#define	__NR_execve	9
#define	__NR_exit	10
#define	__NR_wait4	11
#define __NR_brk	12
#define __NR_reboot	13
#define __NR_chdir	14
#define __NR_getdents	15

extern int syscall(int num, ...);
//TOOD
unsigned long putstring(char *string);
unsigned long open(char *filename, int flags);
unsigned long close(int fd);
unsigned long read(int fd, void * buf, long count);
unsigned long write(int fd, void * buf, long count);
unsigned long lseek(int filds, long offset, int whence);
unsigned long fork();
unsigned long vfork();
unsigned long execve(char *name, char *argv[], char *envp[]);
unsigned long exit(int exit_code);
unsigned long wait4(unsigned long pid, int *status, int options, void *rusage);
unsigned long brk(unsigned long brk);
unsigned long reboot(unsigned long cmd, void * arg);
unsigned long chdir(char *filename);
unsigned long getdents(int fd, void * dirent, long count);

#define PP_FILL(...) PP_FILL_I(__VA_ARGS__, 0, 0, 0, 0, 0, 0, 0)
#define PP_FILL_I(e1, e2, e3, e4, e5, e6, e7, ...) e1, e2, e3, e4, e5, e6, e7
#define syscall(...) syscall(PP_FILL(__VA_ARGS__))

#endif
