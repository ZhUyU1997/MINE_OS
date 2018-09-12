#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

// #define __NR_fork		0
// #define __NR_exec		1
// #define __NR_sbrk		2
// #define __NR_wait		3
// #define __NR_exit		4
// #define __NR_trace		5
// #define __NR_alarm		6
// #define __NR_sigctl		7
// #define __NR_getpid		8
// #define __NR_setgrp		9
// #define __NR_setsid		10
// #define __NR_getuid		11
// #define __NR_setuid		12
// #define __NR_getgid		13
// #define __NR_setgid		14
// #define __NR_access		15
// #define __NR_open		16
// #define __NR_read		17
// #define __NR_write		18
// #define __NR_close		19
// #define __NR_lseek		20
// #define __NR_fcntl		21
// #define __NR_ioctl		22
// #define __NR_pipe		23
// #define __NR_mknod		24
// #define __NR_mkdir		25
// #define __NR_link		26
// #define __NR_rename		27
// #define __NR_unlink		28
// #define __NR_rmdir		29
// #define __NR_chdir		30
// #define __NR_mount		31
// #define __NR_umount		32
// #define __NR_stat		33
// #define __NR_fstat		34
// #define __NR_chmod		35
// #define __NR_chown		36
// #define __NR_utime		37
// #define __NR_sync		38
// #define __NR_getime		39
// #define __NR_setime		40
// #define __NR_sigsend	41
// #define __NR_sigmask	42
// #define __NR_sigact		43
// #define __NR_times		44
// #define __NR_pause		45

#define __NR_open		0
#define __NR_close		1
#define __NR_read		2
#define __NR_write		3
#define __NR_access		4
#define __NR_chdir		5
#define __NR_unlink		6
#define __NR_rename		7
#define __NR_mknod		8
#define __NR_mkdir		9
#define __NR_rmdir		10
#define __NR_getdents	11
#define __NR_stat		12
#define __NR_fstat		13
#define __NR_chown		14
#define __NR_chmod		15
#define __NR_utime		16

#endif
