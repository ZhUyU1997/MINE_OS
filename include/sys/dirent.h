#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H
#include <fs.h>
#include <sys/types.h>

#define MAX_NAME_LEN	255
//与YAFFS_MAX_NAME_LENGTH一致
struct inode;
struct dirent {
	ino_t			d_ino;       /* inode number */
	off_t			d_off;       /* offset to the next dirent */
	unsigned short	d_reclen;    /* length of this record */
	unsigned char	d_type;      /* type of file; not supported
								  by all file system types */
	char			d_name[0]; /* filename */
};

#endif
