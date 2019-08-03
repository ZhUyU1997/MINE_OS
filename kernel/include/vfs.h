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

#ifndef __VFS_H__
#define __VFS_H__

#include "lib.h"
#include <types.h>
#include <block/block.h>

#define O_RDONLY			(1 << 0)
#define O_WRONLY			(1 << 1)
#define O_RDWR				(O_RDONLY | O_WRONLY)
#define O_ACCMODE			(O_RDWR)

#define O_CREAT				(1 << 8)
#define O_EXCL				(1 << 9)
#define O_NOCTTY			(1 << 10)
#define O_TRUNC				(1 << 11)
#define O_APPEND			(1 << 12)
#define O_DSYNC				(1 << 13)
#define O_NONBLOCK			(1 << 14)
#define O_SYNC				(1 << 15)

#define S_IXOTH				(1 << 0)
#define S_IWOTH				(1 << 1)
#define S_IROTH				(1 << 2)
#define S_IRWXO				(S_IROTH | S_IWOTH | S_IXOTH)

#define S_IXGRP				(1 << 3)
#define S_IWGRP				(1 << 4)
#define S_IRGRP				(1 << 5)
#define S_IRWXG				(S_IRGRP | S_IWGRP | S_IXGRP)

#define S_IXUSR				(1 << 6)
#define S_IWUSR				(1 << 7)
#define S_IRUSR				(1 << 8)
#define S_IRWXU				(S_IRUSR | S_IWUSR | S_IXUSR)

#define	S_IFDIR				(1 << 16)
#define	S_IFCHR				(1 << 17)
#define	S_IFBLK				(1 << 18)
#define	S_IFREG				(1 << 19)
#define	S_IFLNK				(1 << 20)
#define	S_IFIFO				(1 << 21)
#define	S_IFSOCK			(1 << 22)
#define	S_IFMT				(S_IFDIR | S_IFCHR | S_IFBLK | S_IFREG | S_IFLNK | S_IFIFO | S_IFSOCK)

#define S_ISDIR(m)			((m) & S_IFDIR )
#define S_ISCHR(m)			((m) & S_IFCHR )
#define S_ISBLK(m)			((m) & S_IFBLK )
#define S_ISREG(m)			((m) & S_IFREG )
#define S_ISLNK(m)			((m) & S_IFLNK )
#define S_ISFIFO(m)			((m) & S_IFIFO )
#define S_ISSOCK(m)			((m) & S_IFSOCK )

#define	R_OK				(1 << 2)
#define	W_OK				(1 << 1)
#define	X_OK				(1 << 0)

struct stat {
	u64_t st_ino;
	s64_t st_size;
	u32_t st_mode;
	u64_t st_dev;
	u32_t st_uid;
	u32_t st_gid;
	u64_t st_ctime;
	u64_t st_atime;
	u64_t st_mtime;
};

struct filesystem_t {
	struct kobj_t * kobj;
	struct list_head list;
	const char * name;
	int fs_flags;
	struct super_block *(*read_superblock)(struct block_t * block);
};

bool_t mount_fs(char * path, char *dev, char * name);
bool_t register_filesystem(struct filesystem_t * fs);
bool_t unregister_filesystem(struct filesystem_t * fs);

struct super_block_operations;
struct inode_operations;
struct dentry_operations;
struct file_operations;

struct super_block {
	struct dentry *root;

	struct super_block_operations *sb_ops;

	void *s_private;
};

struct inode {
	s64_t i_size;
	u32_t i_mode;
	u64_t i_ctime;
	u64_t i_atime;
	u64_t i_mtime;

	struct dentry *i_dentry;
	struct super_block *sb;

	struct file_operations *f_ops;
	struct inode_operations *inode_ops;

	void *i_private;
};

struct vfs_mount_t {
	char m_path[512];
	int mount_type;
	struct super_block  * m_sb;
	struct filesystem_t * m_fs;
	
};

//TODO: fat32文件名能达到256，应设为257 ？？
#define VFS_MAX_PATH	4096
#define VFS_MAX_NAME	257

struct dentry {
	char name[VFS_MAX_NAME];
	struct list_head child_node;
	struct list_head subdirs_list;

	struct inode *d_inode;
	struct dentry *d_parent;

	struct dentry_operations *dir_ops;
	struct vfs_mount_t *vfs_mount;
};

struct file {
	long f_pos;
	unsigned long flags;

	struct dentry *dentry;

	struct file_operations *f_ops;

	void *private_data;
};

struct super_block_operations {
	void (*write_superblock)(struct super_block *sb);
	void (*put_superblock)(struct super_block *sb);

	void (*write_inode)(struct inode *inode);
};

struct inode_operations {
	long (*create)(struct inode *inode, struct dentry *dentry, int mode);
	struct dentry *(*lookup)(struct inode *parent_inode, struct dentry *dest_dentry);
	long (*mkdir)(struct inode *inode, char *name, int mode);
	long (*mknod)(struct inode *inode, char *name, int mode, dev_t dev);
	long (*rmdir)(struct inode *inode, struct dentry *dentry);
	long (*rename)(struct dentry * old_dentry, struct dentry * new_dir_dentry, char *new_name);
	long (*getattr)(struct dentry *dentry, unsigned long *attr);
	long (*setattr)(struct dentry *dentry, unsigned long *attr);
};

struct dentry_operations {
	long (*compare)(struct dentry *parent_dentry, char *source_filename, char *destination_filename);
	long (*hash)(struct dentry *dentry, char *filename);
	long (*release)(struct dentry *dentry);
	long (*iput)(struct dentry *dentry, struct inode *inode);
};

typedef int (*filldir_t)(void *buf, char *name, long namelen, long type, long offset);

struct file_operations {
	long (*read)(struct file *filp, char *buf, unsigned long count, long *position);
	long (*write)(struct file *filp, char *buf, unsigned long count, long *position);
	long (*lseek)(struct file *filp, long offset, long origin);
	long (*ioctl)(struct inode *inode, struct file *filp, unsigned long cmd, unsigned long arg);

	long (*readdir)(struct file *filp, void *dirent, filldir_t filler);
};

struct dentry * path_walk(char * name, unsigned long flags, char *basename);
int fill_dentry(void *buf, char *name, long namelen, long type, long offset);

struct dentry *d_alloc(struct dentry * parent, const char *name);
struct dentry * d_lookup(struct dentry * parent, char *name);
int d_delete(struct dentry * dent);

#endif
