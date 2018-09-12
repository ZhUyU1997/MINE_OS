#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <sys/types.h>

struct stat {
	dev_t   st_dev;		/* Device ID of device containing file.	*/
	ino_t   st_ino;		/* File serial number.			*/
	mode_t  st_mode; 	/* Mode of file (see below).		*/
	nlink_t st_nlink; 	/* Number of hard links to the file.	*/
	uid_t   st_uid; 	/* User ID of file.			*/
	gid_t   st_gid; 	/* Group ID of file.			*/
	dev_t   st_rdev; 	/* Device ID (char/block special file). */
	off_t   st_size; 	/* regular files size in bytes.		*/
	time_t  st_atime; 	/* Time of last access.			*/
	time_t  st_mtime; 	/* Time of last data modification.	*/
	time_t  st_ctime; 	/* Time of last status change.		*/
};

/* Symbolic names for the values of type mode_t */
#define	S_IXOTH		 000001
#define S_IWOTH		 000002
#define S_IROTH		 000004
#define S_TRWXO		 000007

#define S_IXGRP		 000010
#define S_IWGRP		 000020
#define S_IRGRP		 000040
#define S_IRWXG		 000070

#define S_IXUSR		 000100
#define S_IWUSR		 000200
#define S_IRUSR		 000400
#define S_IRWXU		 000700

#define S_ISVIX		 001000
#define S_ISGID		 002000
#define S_ISUID		 004000

#define S_IFREG		0100000
#define S_IFDIR		0040000
#define S_IFCHR		0020000
#define S_IFBLK		0060000
#define S_IFIFO		0010000
#define S_IFLNK		0030000
#define S_IFSOCK	0070000
#define S_IFMT		0170000



#define S_ISREG(m)	(((m)&S_IFMT)==S_IFREG)
#define S_ISDIR(m)	(((m)&S_IFMT)==S_IFDIR)
#define S_ISCHR(m)	(((m)&S_IFMT)==S_IFCHR)
#define S_ISBLK(m)	(((m)&S_IFMT)==S_IFBLK)
#define S_ISFIFO(m)	(((m)&S_IFMT)==S_IFIFO)
#define S_ISLNK(m)	(((m)&S_IFMT)==S_IFLNK)
#define S_ISSOCK(m)	(((m)&S_IFMT)==S_IFSOCK)

//TODO:yportenv.h中加入
#define S_IREAD		0000400
#define	S_IWRITE	0000200
#define	S_IEXEC		0000100


#define XATTR_CREATE	0x1	/* set value, fail if attr already exists */
#define XATTR_REPLACE	0x2	/* set value, fail if attr does not exist */
/* Access and modification times structure */
struct utimebuf {
	time_t atime;
	time_t mtime;
	time_t ctime;
};

extern int stat(char *filename, struct stat *statbuf);
extern int fstat(int fd, struct stat *statbuf);
extern int chown(char *filename, uid_t uid, gid_t gid);
extern int chmod(char *filename, mode_t mode);
extern int utime(char *filename, struct utimebuf *utbuf);

#endif
