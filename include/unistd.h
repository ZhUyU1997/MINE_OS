#ifndef __UNISTD_H__
#define __UNISTD_H__

#define __NR_SYSCALL_BASE		0  

  
/* 
* This file contains the system call numbers. 
*/  
  
#define __NR_restart_syscall	(__NR_SYSCALL_BASE+  0)
#define __NR_exit				(__NR_SYSCALL_BASE+  1)
#define __NR_fork				(__NR_SYSCALL_BASE+  2)
#define __NR_read				(__NR_SYSCALL_BASE+  3)
#define __NR_write				(__NR_SYSCALL_BASE+  4)
#define __NR_open				(__NR_SYSCALL_BASE+  5)
#endif