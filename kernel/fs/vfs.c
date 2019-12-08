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

#include <spinlock.h>
#include "vfs.h"
#include "lib.h"
#include "printk.h"
#include "sys/dirent.h"
#include "errno.h"
#include "memory.h"


CLASS(super_block) {};
CLASS(inode) {};
CLASS(vfs_mount_t) {};
CLASS(dentry) {};
CLASS(file) {};

struct dentry root_dentry = {
	.vfs_mount = NULL,
	.name = "/"
};



struct dentry * path_walk(char * name, unsigned long flags, char *basename) {
	struct dentry * parent = &root_dentry;
	struct dentry * path = parent;

	if (!name || !*name) return NULL;

	while (*name == '/') *name++ = '\0';

	for (;;) {
		char *tmpname = name;

		while (*name && (*name != '/'))
			name++;

		int len = name - tmpname;

		while (*name == '/') *name++ = '\0';

		if(!len)
			break;

		if (len >= VFS_MAX_NAME)
			return NULL;

		if (!*name) {
			if (flags & 1) {
				strlcpy(basename, tmpname, VFS_MAX_NAME);
				return parent;
			}
		}

		path = d_lookup(parent, tmpname);

		if (path == NULL) {
			path = d_alloc(parent, tmpname);
			if (path == NULL)
				return NULL;

			struct inode *inode = parent->d_inode;
			if (inode->inode_ops->lookup(inode, path)) {
				//color_printk(RED, WHITE, "can not find file or dir:%s", path->name);
				kfree(path);
				return NULL;
			}
			list_add(&path->child_node, &parent->subdirs_list);
		}

		parent = path;
	}

	return path;
}

int fill_dentry(void *buf, char *name, long namelen, long type, long offset) {
	struct dirent* dent = (struct dirent*)buf;
	int reclen = ALIGN(sizeof(struct dirent) + namelen + 1, sizeof(int));////TODO:可能多加一点，万一宽字符

	if ((unsigned long)buf < CONFIG_TASK_SIZE && !verify_area(buf, sizeof(struct dirent) + namelen))
		return -EFAULT;

	memcpy(dent->d_name, name, reclen);
	dent->d_reclen = reclen;
	dent->d_type = type;
	dent->d_off = offset;
	return sizeof(struct dirent) + reclen;
}


static struct list_head __filesystem_list = {
	.next = &__filesystem_list,
	.prev = &__filesystem_list,
};
static spinlock_t __filesystem_lock = SPIN_LOCK_INIT();

static struct kobj_t * search_class_filesystem_kobj(void)
{
	struct kobj_t * kclass = kobj_search_directory_with_create(kobj_get_root(), "class");
	return kobj_search_directory_with_create(kclass, "filesystem");
}

struct filesystem_t * search_filesystem(const char * name)
{
	struct filesystem_t * pos, * n;

	if(!name)
		return NULL;

	list_for_each_entry_safe(pos, n, &__filesystem_list, list)
	{
		if(strcmp(pos->name, name) == 0)
			return pos;
	}
	return NULL;
}


bool_t mount_fs(char * path, char *dev, char * name) {
	struct block_t * bdev = NULL;
	struct filesystem_t * fs = search_filesystem(name);
	if (!fs)
		return FALSE;

	if(dev)
		while(!(bdev = search_block(dev)));

	struct super_block *sb = fs->read_superblock(bdev);
	if(!strcmp(path, "/"))
	{
		if(root_dentry.vfs_mount)
			return FALSE;
		init_list_head(&root_dentry.child_node);
		init_list_head(&root_dentry.subdirs_list);
	}
	
	struct dentry * dent = path_walk(path, 0, NULL);
	if(!dent) {
		printf("failed to mount %s on %s\n", name, path);
		return FALSE;
	}
	//TODO:dent的child计数为零才可以mount
	struct vfs_mount_t *mount = calloc(1, sizeof(struct vfs_mount_t));

	mount->m_fs = fs;
	mount->m_sb = sb;

	dent->vfs_mount = mount;
	dent->d_inode = sb->root;
	return TRUE;
}

bool_t register_filesystem(struct filesystem_t * fs)
{
	irq_flags_t flags;

	if(!fs || !fs->name)
		return FALSE;

	if(search_filesystem(fs->name))
		return FALSE;

	fs->kobj = kobj_alloc_directory(fs->name);
	kobj_add(search_class_filesystem_kobj(), fs->kobj);

	spin_lock_irqsave(&__filesystem_lock, flags);
	list_add_tail(&fs->list, &__filesystem_list);
	spin_unlock_irqrestore(&__filesystem_lock, flags);

	return TRUE;
}

bool_t unregister_filesystem(struct filesystem_t * fs)
{
	irq_flags_t flags;

	if(!fs || !fs->name)
		return FALSE;

	spin_lock_irqsave(&__filesystem_lock, flags);
	list_del(&fs->list);
	spin_unlock_irqrestore(&__filesystem_lock, flags);
	kobj_remove(search_class_filesystem_kobj(), fs->kobj);
	kobj_remove_self(fs->kobj);

	return TRUE;
}


