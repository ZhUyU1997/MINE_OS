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


#include "errno.h"
#include "printk.h"
#include "lib.h"
#include "vfs.h"
#include "fcntl.h"
#include "memory.h"
#include "stdio.h"
#include "fcntl.h"
#include "ptrace.h"
#include "task.h"
#include "sched.h"
#include "memory.h"
#include "sys.h"
#include "sys/err.h"

char * getname(char *name) {
	char *path = (char *)kzalloc(PAGE_4K_SIZE, 0);
	if (path == NULL)
		return -ENOMEM;

	int len = strnlen_user(name, PAGE_4K_SIZE);
	if (len <= 0) {
		kfree(path);
		//TODO: 应该返回ENOENT？
		return -ENOENT;
	} else if (len >= PAGE_4K_SIZE) {
		kfree(path);
		return -ENAMETOOLONG;
	}
	strncpy_from_user(path, name, len);

	return path;
}
void putname(char *name) {
	kfree(name);
}

unsigned long no_system_call(void) {
	color_printk(RED, BLACK, "no_system_call is calling");
	return -ENOSYS;
}

unsigned long sys_putstring(char *string) {
	color_printk(ORANGE, WHITE, "%s", string);
	return 0;
}
unsigned long sys_mknod(const char *filename, u32_t mode, dev_t dev) {
	char * path = NULL;
	struct dentry * dentry = NULL;
	int ret = -1;

	char basename[VFS_MAX_NAME] = {0};
	
	if (S_ISDIR(mode) || S_ISLNK(mode))
		return -EINVAL;

	
	path = getname(filename);
	
	if(IS_ERR(path)) 
		return PTR_ERR(path);

	dentry = path_walk(path, 1, basename);
	putname(path);

	if(!dentry) {
		return -ENOENT; 
	}

	if (dentry->d_inode->inode_ops && dentry->d_inode->inode_ops->mknod)
		ret = dentry->d_inode->inode_ops->mknod(dentry->d_inode, basename, mode, dev);
	return ret;
}
unsigned long sys_open(const char *filename, u32_t flags, u32_t mode) {
	char * path = NULL;
	long error = 0;
	struct dentry * dentry = NULL;
	struct file * filp = NULL;
	struct file ** f = NULL;
	int fd = -1;
	int i;
	int ret = -1;

//	color_printk(GREEN,BLACK,"sys_open");
	path = getname(filename);
	if(IS_ERR(path)) 
		return PTR_ERR(path);

	dentry = path_walk(path, 0, NULL);
	putname(path);

	if (dentry == NULL) {
		if (!(flags & O_CREAT)) {
			return -ENOENT;
		}
	
		if (sys_mknod(filename, S_IFREG | (mode & 0777), 0) < 0){
			putname(path);
			return -EAGAIN;
		}

		path = getname(filename);
		if(IS_ERR(path)) 
			return PTR_ERR(path);

		dentry = path_walk(path, 0, NULL);
		putname(path);
		if (dentry == NULL) {
			return -EAGAIN;
		}
	} else if ((flags & O_EXCL) && (flags & O_CREAT)) { //TODO
		return -EEXIST;
	}

	filp = (struct file *)kzalloc(sizeof(struct file), 0);

	filp->dentry = dentry;
	filp->flags = flags;
	filp->f_ops = dentry->d_inode->f_ops;

	if (filp->flags & O_TRUNC) {
		//TODO:清除磁盘数据
		filp->dentry->d_inode->i_size = 0;
	}
	if (filp->flags & O_APPEND) {
		filp->f_pos = filp->dentry->d_inode->i_size;
	}

	f = current->file_struct;
	for (i = 3; i < TASK_FILE_MAX; i++)
		if (f[i] == NULL) {
			fd = i;
			break;
		}
	if (i == TASK_FILE_MAX) {
		kfree(filp);
		//// reclaim struct index_node & struct dir_entry
		return -EMFILE;
	}
	f[fd] = filp;

	return fd;
}
unsigned long sys_close(int fd) {
	struct file * filp = NULL;

//	color_printk(GREEN,BLACK,"sys_close:%d",fd);
	if (fd < 0 || fd >= TASK_FILE_MAX)
		return -EBADF;

	filp = current->file_struct[fd];

	kfree(filp);
	current->file_struct[fd] = NULL;

	return 0;
}
unsigned long sys_read(int fd, void * buf, long count) {
	struct file * filp = NULL;
	unsigned long ret = 0;

//	color_printk(GREEN,BLACK,"sys_read:%d",fd);
	if (fd < 0 || fd >= TASK_FILE_MAX)
		return -EBADF;
	if (count < 0)
		return -EINVAL;

	filp = current->file_struct[fd];
	if (filp->f_ops && filp->f_ops->read)
		ret = filp->f_ops->read(filp, buf, count, &filp->f_pos);
	return ret;
}
unsigned long sys_write(int fd, void * buf, long count) {
	struct file * filp = NULL;
	unsigned long ret = 0;

//	color_printk(GREEN,BLACK,"sys_write:%d",fd);
	if (fd < 0 || fd >= TASK_FILE_MAX)
		return -EBADF;
	if (count < 0)
		return -EINVAL;

	filp = current->file_struct[fd];
	if (filp->f_ops && filp->f_ops->write)
		ret = filp->f_ops->write(filp, buf, count, &filp->f_pos);
	return ret;
}
unsigned long sys_lseek(int fd, long offset, int whence) {
	struct file * filp = NULL;
	unsigned long ret = 0;

//	color_printk(GREEN,BLACK,"sys_lseek:%d",filds);
	if (fd < 0 || fd >= TASK_FILE_MAX)
		return -EBADF;
	if (whence < 0 || whence > SEEK_END)
		return -EINVAL;

	filp = current->file_struct[fd];
	if (filp->f_ops && filp->f_ops->lseek)
		ret = filp->f_ops->lseek(filp, offset, whence);
	return ret;
}
unsigned long sys_fork(struct pt_regs *regs) {
	color_printk(GREEN, BLACK, "sys_fork");
	return do_fork(regs, 0, regs->ARM_sp, 0);
}
unsigned long sys_vfork(struct pt_regs *regs) {
	color_printk(GREEN, BLACK, "sys_vfork");
	return do_fork(regs, CLONE_VM | CLONE_FS | CLONE_SIGHAND, regs->ARM_sp, 0);
}
unsigned long sys_execve(struct pt_regs *regs) {
	char * path = NULL;
	long error = 0;

	color_printk(GREEN, BLACK, "sys_execve");
	path = getname((char *)regs->ARM_r0);
	if(IS_ERR(path)) 
		return PTR_ERR(path);

	error = do_execve(regs, path, (char **)regs->ARM_r1, NULL);
	putname(path);

	return error;
}
unsigned long sys_exit(int exit_code) {
	color_printk(GREEN, BLACK, "sys_exit");
	return do_exit(exit_code);
}
unsigned long sys_wait4(unsigned long pid, int *status, int options, void *rusage) {
	long retval = 0;
	struct task_t *child = NULL;
	struct task_t *tsk = NULL;

	color_printk(GREEN, BLACK, "sys_wait4");
	
	struct list_head *pos;
	list_for_each(pos, &current->child_list) {
		struct task_t *temp = list_entry(pos, struct task_t, child_node);
		if (temp->pid == pid) {
			child = temp;
			break;
		}
	}

	if (child == NULL)
		return -ECHILD;
	if (options != 0)
		return -EINVAL;

	if (child->status == TASK_STATUS_ZOMBIE) {
		//TODO
		copy_to_user(status, &child->exit_code, sizeof(int));
		list_del(&tsk->child_node);
		exit_mm(child);
		task_destroy(child);
		return retval;
	}

	sleep_on(&current->wait_childexit);

	copy_to_user(status, &child->exit_code, sizeof(long));
	list_del(&tsk->child_node);
	exit_mm(child);
	task_destroy(child);
	return retval;
}
unsigned long sys_brk(unsigned long brk) {
	unsigned long new_brk = PAGE_2M_ALIGN(brk);

//	color_printk(GREEN,BLACK,"sys_brk");
//	color_printk(RED,BLACK,"brk:%#018lx,new_brk:%#018lx,current->mm->end_brk:%#018lx",brk,new_brk,current->mm->end_brk);
	if (new_brk == 0)
		return current->mm->start_brk;
	if (new_brk < current->mm->end_brk)	//release  brk space
		return 0;

	new_brk = do_brk(current->mm->end_brk, new_brk - current->mm->end_brk);	//expand brk space

	current->mm->end_brk = new_brk;
	return new_brk;
}
unsigned long sys_reboot(unsigned long cmd, void * arg) {
	color_printk(GREEN, BLACK, "sys_reboot");
	switch (cmd) {
		case SYSTEM_REBOOT:
			color_printk(RED, BLACK, "sys_reboot cmd SYSTEM_REBOOT!");
			break;
		case SYSTEM_POWEROFF:
			color_printk(RED, BLACK, "sys_reboot cmd SYSTEM_POWEROFF");
			break;

		default:
			color_printk(RED, BLACK, "sys_reboot cmd ERROR!");
			break;
	}
	return 0;
}
unsigned long sys_chdir(char *filename) {
	char * path = NULL;
	struct dentry * dentry = NULL;

	color_printk(GREEN, BLACK, "sys_chdir");

	path = getname(filename);
	if(IS_ERR(path)) 
		return PTR_ERR(path);

	dentry = path_walk(path, 0, NULL);
	putname(path);

	if (dentry == NULL)
		return -ENOENT;
	if (!S_ISDIR(dentry->d_inode->i_mode))
		return -ENOTDIR;
	return 0;
}
unsigned long sys_getdents(int fd, void * dirent, long count) {
	struct file * filp = NULL;
	unsigned long ret = 0;

	//color_printk(GREEN,BLACK,"sys_getdents:%d",fd);
	if (fd < 0 || fd > TASK_FILE_MAX)
		return -EBADF;
	if (count < 0)
		return -EINVAL;

	filp = current->file_struct[fd];
	if (filp->f_ops && filp->f_ops->readdir)
		ret = filp->f_ops->readdir(filp, dirent, &fill_dentry);
	return ret;
}
static void copy_stat(struct inode *inode , struct stat *statbuf) {
	statbuf->st_mode = inode->i_mode;
	statbuf->st_size = inode->i_size;
	statbuf->st_atime = inode->i_atime;
	statbuf->st_mtime = inode->i_mtime;
	statbuf->st_ctime = inode->i_ctime;
}
unsigned long sys_stat(char *pathname, struct stat *statbuf) {
	char * path = NULL;
	struct dentry * dentry = NULL;

	path = getname(pathname);
	if(IS_ERR(path)) 
		return PTR_ERR(path);

	dentry = path_walk(path, 0, NULL);
	putname(path);

	if (dentry == NULL)
		return -ENOENT;
	copy_stat(dentry->d_inode , statbuf);
	return 0;
}
unsigned long sys_fstat(int fd, struct stat *statbuf) {
	return 0;
}
unsigned long sys_mkdir(const char *filename, u32_t mode) {
	char * path = NULL;
	struct dentry * dentry = NULL;
	int ret = -1;

	char basename[VFS_MAX_NAME] = {0};

	path = getname(filename);
	if(IS_ERR(path)) 
		return PTR_ERR(path);

	dentry = path_walk(path, 1, basename);
	putname(path);

	if(!dentry) {
		return -ENOENT; 
	}
	
	mode = S_IFDIR | mode;

	if (dentry->d_inode->inode_ops && dentry->d_inode->inode_ops->mkdir)
		ret = dentry->d_inode->inode_ops->mkdir(dentry->d_inode, basename, mode);
	return ret;
}
unsigned long sys_rmdir(const char *filename) {
	char * path = NULL;
	struct dentry * dentry = NULL;
	int ret = -1;
	char basename[VFS_MAX_NAME] = {0};

	path = getname(filename);
	if(IS_ERR(path)) 
		return PTR_ERR(path);

	dentry = path_walk(path, 0, NULL);
	putname(path);

	if(!dentry) {
		return -ENOENT; 
	}

	struct inode_operations * ops = dentry->d_inode->inode_ops;
	if (ops && ops->rmdir)
		ret = ops->rmdir(dentry->d_parent->d_inode, dentry);

	return ret;
}
unsigned long sys_rename(const char *old, const char *new) {
	char * path = NULL;
	long pathlen = 0;
	struct dentry * old_dentry, * new_dir_dentry;
	int ret = -1;
	char basename[VFS_MAX_NAME] = {0};

	path = getname(old);
	if(IS_ERR(path)) 
		return PTR_ERR(path);

	old_dentry = path_walk(path, 0, NULL);
	putname(path);
	if(!old_dentry) {
		return -ENOENT; 
	}

	path = getname(new);
	if(IS_ERR(path)) 
		return PTR_ERR(path);

	new_dir_dentry = path_walk(path, 1, basename);
	putname(path);

	if(!old_dentry) {
		return -ENOENT; 
	}
	struct inode_operations * ops = new_dir_dentry->d_inode->inode_ops;
	if (ops && ops->rename)
		ret = ops->rename(old_dentry, new_dir_dentry, basename);

	return ret;
}


