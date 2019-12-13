/*
 * kernel/vfs/sys/sys.c
 *
 * Copyright(c) 2007-2019 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <printk.h>
#include <vfs.h>
#include <errno.h>
#include <stdio.h>
#include <core/initcall.h>
#include <block/block.h>
#include <class.h>

CLASS_DEF(sys_inode)
{
	struct kobj_t *obj;
};

CLASS(sys_inode, inode) {};

static long sys_read(struct file * filp, char * buf, size_t count, off_t * pos)
{
	struct kobj_t * kobj;
	struct inode * n = filp->dentry->d_inode;

	if(!S_ISREG(n->i_mode))
		return -1;

	kobj = dynamic_cast(sys_inode)(n)->obj;
	if(*pos == 0)
	{
		if(kobj && kobj->read)
			return *pos = kobj->read(kobj, buf, count);
	}
	return 0;
}

static long sys_write(struct file * filp, char * buf, size_t count, off_t * pos)
{
	struct kobj_t * kobj;
	struct inode * n = filp->dentry->d_inode;

	if(!S_ISREG(n->i_mode))
		return -1;

	kobj = dynamic_cast(sys_inode)(n)->obj;
	if(*pos == 0)
	{
		if(kobj && kobj->write)
			return *pos = kobj->write(kobj, buf, count);
	}
	return 0;
}

static long sys_lseek(struct file * filp, long offset, long origin) {
	//struct index_node *inode = filp->dentry->dir_inode;
	long pos = 0;

	switch (origin) {
		case SEEK_SET:
		case SEEK_CUR:
		case SEEK_END:
			filp->f_pos = 0;
			break;

		default:
			return -EINVAL;
	}
	filp->f_pos = pos;
	return pos;
}

static long sys_ioctl(struct inode * inode, struct file * filp, unsigned long cmd, unsigned long arg) {
	return -1;
}

static long sys_readdir(struct file * filp, void * dirent, filldir_t filler)
{
	struct kobj_t * kobj, * obj;
	struct list_head * pos;
	off_t off;
	int namelen = 0;

	off = filp->f_pos;
	kobj = dynamic_cast(sys_inode)(filp->dentry->d_inode)->obj;
	if(list_empty(&kobj->children))
		return -1;

	pos = (&kobj->children)->next;
	for(int i = 0; i != off; i++)
	{
		pos = pos->next;
		if(pos == (&kobj->children))
			return -1;
	}
	filp->f_pos++;
	obj = list_entry(pos, struct kobj_t, entry);

	namelen = strlen(obj->name);
	return filler(dirent, obj->name, namelen, 0, 0);
}


static struct file_operations file_ops = {
	.read = sys_read,
	.write = sys_write,
	.lseek = sys_lseek,
	.ioctl = sys_ioctl,

	.readdir = sys_readdir,
};


static long sys_create(struct inode * inode, struct dentry * dentry, int mode) {
	return -1;
}
static long sys_mkdir(struct inode * parent_inode, char * name, int mode) {
	return -1;
}
static long sys_mknod(struct inode * parent_inode, char * name, int mode, dev_t dev) {
	return -1;
}
static long sys_rmdir(struct inode * parent_inode, struct dentry * dentry) {
	return -1;
}
static long sys_rename(struct dentry * old_dentry, struct dentry * new_dir_dentry, char *new_name) {
	return -1;
}
static long sys_getattr(struct dentry * dentry, unsigned long * attr) {
	return -1;
}
static long sys_setattr(struct dentry * dentry, unsigned long * attr) {
	return -1;
}

static long sys_lookup(struct inode * parent_inode, struct dentry * dest_dentry);
static struct inode_operations inode_ops = {
	.create = sys_create,
	.lookup = sys_lookup,
	.mkdir = sys_mkdir,
	.mknod = sys_mknod,
	.rmdir = sys_rmdir,
	.rename = sys_rename,
	.getattr = sys_getattr,
	.setattr = sys_setattr,
};

static long sys_lookup(struct inode * parent_inode, struct dentry * dest_dentry)
{
	struct kobj_t * kobj, * obj;

	if(*dest_dentry->name == '\0')
		return -1;

	kobj = dynamic_cast(sys_inode)(parent_inode)->obj;
	obj = kobj_search(kobj, dest_dentry->name);
	if(!obj)
		return -1;

	sys_inode * sn = NEW(sys_inode);
	inode *n = dynamic_cast(inode)(sn);
	n->i_atime = 0;
	n->i_mtime = 0;
	n->i_ctime = 0;
	n->i_mode = 0;
	n->i_size = 0;
	sn->obj = obj;

	if(obj->type == KOBJ_TYPE_DIR)
	{
		n->i_mode |= S_IFDIR;
		n->i_mode |= S_IRWXU | S_IRWXG | S_IRWXO;
	}
	else
	{
		n->i_mode |= S_IFREG;
		if(obj->read)
			n->i_mode |= (S_IRUSR | S_IRGRP | S_IROTH);
		if(obj->write)
			n->i_mode |= (S_IWUSR | S_IWGRP | S_IWOTH);
	}
	n->i_dentry = dest_dentry;
	n->sb = parent_inode->sb;
	n->f_ops = &file_ops;
	n->inode_ops = &inode_ops;
	dest_dentry->d_inode = n;
	return 0;
}

static void write_superblock(struct super_block * sb) {}

static void put_superblock(struct super_block * sb) {
	kfree(sb->root);
	kfree(sb);
}

static void write_inode(struct inode * inode) {}

struct super_block_operations sb_ops = {
	.write_superblock = write_superblock,
	.put_superblock = put_superblock,

	.write_inode = write_inode,
};

static struct super_block * read_superblock(struct block_t * block) {
	struct super_block * sbp = NEW(super_block);
	sbp->sb_ops = &sb_ops;

	struct sys_inode *si = NEW(sys_inode);
	struct inode * in = dynamic_cast(inode)(si);
	in->inode_ops = &inode_ops;
	in->f_ops = &file_ops;
	in->i_size = 0;
	in->i_mode |= S_IFDIR;
	in->sb = sbp;
	in->i_dentry = NULL;
	si->obj = kobj_get_root();
	sbp->root = in;

	return sbp;
}

struct filesystem_t sys = {
	.name = "sys",
	.fs_flags = 0,
	.read_superblock = read_superblock,
};

static __init void filesystem_sys_init(void)
{
	register_filesystem(&sys);
}

static __exit void filesystem_sys_exit(void)
{
	unregister_filesystem(&sys);
}

core_initcall(filesystem_sys_init);
core_exitcall(filesystem_sys_exit);
