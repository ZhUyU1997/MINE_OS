#ifndef _SYS_SIGNAL_H
#define _SYS_SIGNAL_H

#define	NR_SIG		32

#define SIGHUP		1
#define SIGINT		2
#define SIGQUIT		3
#define SIGILL		4
#define SIGTRAP		5
#define SIGABRT		6
#define SIGFPU		7
#define SIGKILL		8
#define SIGUSR1		9
#define SIGSEGV		10
#define SIGUSR2		11
#define SIGPIPE		12
#define SIGALARM	13
#define SIGTREM		14
#define SIGCHLD		15
#define SIGTKFLT	16
#define SIGCONT		17
#define SIGSTOP		18
#define SIGTSTP		19
#define SIGTTIN		20
#define	SIGTTOU		21
#define SIGTERM		22

typedef unsigned long 	  sigset_t;
typedef void        	(*sigact_t)(int signo, int ret);

#define SIG_BLOCK	1
#define SIG_UNBLOCK	2
#define SIG_SETMASK	3

#define SIG_ERR		((sigact_t)-1)
#define SIG_DFL		((sigact_t)0)
#define SIG_IGN		((sigact_t)1)

#define signal(a,b)

#endif
