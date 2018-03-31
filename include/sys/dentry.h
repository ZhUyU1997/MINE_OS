#ifndef _SYS_DENTRY_H
#define _SYS_DENTRY_H
#include <fs.h>
#include <sys/types.h>

#define MAX_NAME_LEN	255
//与YAFFS_MAX_NAME_LENGTH一致
struct inode;
struct dentry{
	struct inode *d_inode;
	struct{
		char name[MAX_NAME_LEN+1];
	}d_name;
};

#endif
