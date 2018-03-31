#ifndef _SYS_UNISTD_H
#define _SYS_UNISTD_H

#include <sys/types.h>
#include <io.h>


/* standard I/O fileno */
#define STDIN_FILENO	0	/* file descriptor of stdin  */
#define STDOUT_FILENO	1	/* file descriptor of stdout */
#define STDERR_FILENO	2	/* file descriptor of stderr */

#define PIPE_READ_ID	0
#define PIPE_WRITE_ID	1

extern char **environ;

extern pid_t fork();
extern pid_t getpid();
extern pid_t getppid();
extern pid_t setgrp();
extern pid_t setsid();
extern uid_t getuid();
extern gid_t getgid();
extern uid_t setuid(uid_t uid);
extern gid_t setgid(gid_t gid);
extern ssize_t read(int fd, void * buf, size_t size);
extern ssize_t write(int fd, void * buf, size_t size);
extern int execve(char *fname, char **argv, char **envp);
extern void _exit(int status);
extern int sbrk(int inc);
extern int dup(int fd);
extern int dup2(int fd, int fd2);
extern int chdir(char *dirname);
extern int close(int fd);
extern int lseek(int fd, long off, int where);
extern int mkdir(char *name, mode_t mode);
extern int rmdir(char *name);
extern int link(const char *name, const char *newname);
extern int sync(void);
extern int unlink(const char *name);
extern int mknod(char *name, mode_t mode, dev_t dev);
extern int execvpe(char * file, char ** argv, char ** envp);
extern int pipe(int fd[2]);
extern int mount(char* special, char *target, char* type, int rw_flag);
extern int umount(char* special);

#endif
