/* This file is part of The Firekylin Operating System.
 *
 * Copyright (c) 2016, Liuxiaofeng
 * All rights reserved.
 *
 * This program is free software; you can distribute it and/or modify
 * it under the terms of The BSD License, see LICENSE.
 */

#ifndef _FS_H
#define _FS_H
#include <sys/types.h>
#include <sys/param.h>
#include "yaffs_list.h"
struct inode {
	struct list_head	i_list;
	dev_t			i_dev;
	ino_t			i_ino;
	mode_t			i_mode;
	nlink_t			i_nlink;
	uid_t 		    i_uid;
	gid_t 		    i_gid;
	dev_t		    i_rdev;
	off_t 		    i_size;
	time_t 		    i_atime;
	time_t 		    i_mtime;
	time_t 		    i_ctime;
	u32						i_version;/* 版本号码 */ //u32原先为u64
	unsigned short			i_count;
	struct fs_operation 	*i_op;
	void					*i_private;
	struct super_block		*i_sb;
	unsigned int			i_blocks;
	unsigned int			i_flags;
	unsigned long			i_state; /* inode目前的状态，可以是I_DIRTY，I_LOCK和 I_FREEING的OR组合 */
};
/* Bits of inode->i_flag */
#define I_NEW		0x0001
#define I_VALID		0x0002
#define I_DIRTY		0x0004
#define I_MOUNT		0x0008
struct path {
	struct dentry *dentry;
};
struct file {
	struct path			f_path;
	loff_t				f_pos;
	unsigned long		f_mode;
	unsigned long		f_count;
	unsigned int		f_flags;
	struct inode		*f_inode;
	struct dentry		*f_dentry;
	u32					f_version;//u32原先为u64
	struct fs_operation	*op;
};

struct super_block {  
    struct list_head	s_list;     /* Keep this first */  
    dev_t				s_dev;      /* search index; _not_ kdev_t */  
    struct list_head	s_inodes;   /* all inodes */  
    struct list_head	s_dirty;    /* dirty inodes */  
    struct list_head	s_io;       /* parked for writeback */  
    struct list_head	s_files;
	void				*s_fs_info; /* Filesystem private info */  
};
#include <sys/dentry.h>
typedef int (*filldir_t)(void *, const char *, int, loff_t, u32, unsigned);//u32原先为u64
struct fs_operation{
	/* super operation */
	int (*super_read)(struct super *super);
	int (*super_write)(struct super *super);
	int (*evict_inode)(struct inode *inode);
	/* inode operation */
	int (*inode_read)(struct inode *inode);
	int (*inode_write)(struct inode *inode);
	int (*lookup)(struct inode *inode, char *filename, struct inode **res);
	int (*mknod)(struct inode *inode, char *name,mode_t mode,dev_t dev);
	int (*mkdir)(struct inode *dir_inode,char *basename,mode_t mode);
	int (*link)(struct inode *dir_inode,char *name,struct inode *inode);
	int (*unlink)(struct inode *dir_inode,char *basename);
	int (*rmdir)(struct inode *dir_inode,char *basename);
	int (*rename)(struct inode *old_inode,char *old, struct inode *new_inode, char *new);

	/* file operation */
	int (*file_read)(struct file *file, char *buf, size_t size, loff_t *pos);
	int (*file_readdir)(struct file *file, void *dirent, filldir_t filldir);
	int (*file_write)(struct file *file, char *buf, size_t size, loff_t *pos);
	int (*file_release)(struct file *file);
};
/*
 * Attribute flags.  These should be or-ed together to figure out what
 * has been changed!
 */
#define ATTR_MODE	(1 << 0)
#define ATTR_UID	(1 << 1)
#define ATTR_GID	(1 << 2)
#define ATTR_SIZE	(1 << 3)
#define ATTR_ATIME	(1 << 4)
#define ATTR_MTIME	(1 << 5)
#define ATTR_CTIME	(1 << 6)
#define ATTR_ATIME_SET	(1 << 7)
#define ATTR_MTIME_SET	(1 << 8)
#define ATTR_FORCE	(1 << 9) /* Not a change, but a change it */
#define ATTR_ATTR_FLAG	(1 << 10)
#define ATTR_KILL_SUID	(1 << 11)
#define ATTR_KILL_SGID	(1 << 12)
#define ATTR_FILE	(1 << 13)
#define ATTR_KILL_PRIV	(1 << 14)
#define ATTR_OPEN	(1 << 15) /* Truncating from open(O_TRUNC) */
#define ATTR_TIMES_SET	(1 << 16)
//struct iattr {
	//unsigned int	ia_valid;
	//mode_t		ia_mode;
	// kuid_t		ia_uid;
	// kgid_t		ia_gid;
	//off_t		ia_size;
	// struct timespec	ia_atime;
	// struct timespec	ia_mtime;
	// struct timespec	ia_ctime;

	/*
	 * Not an attribute, but an auxiliary info for filesystems wanting to
	 * implement an ftruncate() like method.  NOTE: filesystem should
	 * check for (ia_valid & ATTR_FILE), and not for (ia_file != NULL).
	 */
	//struct file	*ia_file;
//};
struct open_intent {  
    int flags;/*标志*/  
    int create_mode;/*创建模式*/  
    struct file *file;/*文件对象指针*/  
}; 
struct nameidata {  
    struct dentry   *dentry;/*当前目录项对象*/    
    unsigned int    flags;/*查询标志*/  
    unsigned    depth;/*当前符号链接的深度，一般小于6*/  
    char *saved_names[255 + 1];/*关联符号链接的路径名数组*/  
  
    /* Intent data */  
    union {  
        struct open_intent open;/*想要打开的文件的联合体*/  
    } intent;  
};
static inline struct inode * idup(struct inode *inode)
{
	//require_lock(&inode->i_lock);
	inode->i_count++;
	return inode;
}
#define DT_UNKNOWN	0
#define DT_FIFO		1
#define DT_CHR		2
#define DT_DIR		4
#define DT_BLK		6
#define DT_REG		8
#define DT_LNK		10
#define DT_SOCK		12
#define DT_WHT		14

struct iattr {
	unsigned int ia_valid;
	unsigned ia_mode;
	unsigned ia_uid;
	unsigned ia_gid;
	unsigned ia_size;
	unsigned ia_atime;
	unsigned ia_mtime;
	unsigned ia_ctime;
	unsigned int ia_attr_flags;
};

#include <sys/dirent.h>
extern struct inode  * root_inode;
extern struct inode  * iget(dev_t dev, ino_t ino);
extern void            iput(struct inode *inode);
extern struct inode  * namei(char *path, char **basename);
#endif
