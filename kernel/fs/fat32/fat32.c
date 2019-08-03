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


#include <printk.h>
#include <task.h>
#include <lib.h>
#include <vfs.h>
#include <errno.h>
#include <stdio.h>
#include <block/block.h>
#include <core/initcall.h>
#include "fat32.h"

static s64_t fatfs_wallclock_mktime(unsigned int year0, unsigned int mon0, unsigned int day, unsigned int hour, unsigned int min, unsigned int sec)
{
	unsigned int year = year0, mon = mon0;
	u64_t ret;

	if(0 >= (int)(mon -= 2))
	{
		mon += 12;
		year -= 1;
	}

	ret = (u64_t)(year / 4 - year / 100 + year / 400 + 367 * mon / 12 + day);
	ret += (u64_t)(year) * 365 - 719499;

	ret *= (u64_t)24;
	ret += hour;

	ret *= (u64_t)60;
	ret += min;

	ret *= (u64_t)60;
	ret += sec;

	return (s64_t)ret;
}

u32_t fatfs_pack_timestamp(struct fat_date date, struct fat_time time)
{
	return (u32_t)fatfs_wallclock_mktime(1980 + date.year, date.month, date.day, time.hours, time.minutes, time.seconds);
}

long FAT32_read(struct file * filp, char * buf, size_t count, off_t *pos) {
	return fat32_node_read(filp->dentry->d_inode, buf, count, pos);
}

bool_t fat32_find_available_cluster(struct fat32_sb_info * fsbi, u32_t *fat_entry) {
	u32_t sector_per_fat = fsbi->sectors_per_fat;
	u32_t buf[128];

//	fsbi->fat_fsinfo->FSI_Free_Count & fsbi->fat_fsinfo->FSI_Nxt_Free not exactly,so unuse

	for (int i = 0; i < sector_per_fat; i++) {
		memset(buf, 0, 512);
		if(!block_read(fsbi->bdev, buf, (fsbi->first_fat_sector + i) * fsbi->bytes_per_sector, fsbi->bytes_per_sector))
			return FALSE;

		for (int j = 0; j < 128; j++) {
			if ((buf[j] & 0x0fffffff) == 0) {
				*fat_entry =  (i << 7) + j;
				return TRUE;
			}
		}
	}
	return FALSE;
}


long FAT32_write(struct file * filp, char * buf, size_t count, off_t * pos) {
	struct inode * inode = filp->dentry->d_inode;
	int ret = fat32_node_write(inode, buf, count, pos);

	if (*pos > inode->i_size) {
		inode->i_size = *pos;
		inode->sb->sb_ops->write_inode(inode);
	}

	return ret;
}


long FAT32_lseek(struct file * filp, long offset, long origin) {
	//struct index_node *inode = filp->dentry->dir_inode;
	long pos = 0;

	switch (origin) {
		case SEEK_SET:
			pos = offset;
			break;

		case SEEK_CUR:
			pos =  filp->f_pos + offset;
			break;

		case SEEK_END:
			pos = filp->dentry->d_inode->i_size + offset;
			break;

		default:
			return -EINVAL;
			break;
	}

	if (pos < 0 || pos > filp->dentry->d_inode->i_size) {
		color_printk(GREEN, BLACK, "FAT32 FS(lseek) ERROR!! position:%d\n", filp->f_pos);
		return -EOVERFLOW;
	}

	filp->f_pos = pos;

	return pos;
}


long FAT32_ioctl(struct inode * inode, struct file * filp, unsigned long cmd, unsigned long arg) {
	return -1;
}

long FAT32_readdir(struct file * filp, void * dirent, filldir_t filler) {
	struct fat32_inode_info * finode = filp->dentry->d_inode->i_private;
	struct fat32_sb_info * fsbi = filp->dentry->d_inode->sb->s_private;

	char name[VFS_MAX_NAME] = {0};

	if(fat32_node_read_dirent_name(filp->dentry->d_inode, name, &filp->f_pos))
		return NULL;

	int namelen = strlen(name);
	return filler(dirent, name, namelen, 0, 0);
}

bool_t fat_dir_is_empty(struct inode *inode) {
	char name[VFS_MAX_NAME] = {0};
	off_t pos=0;
	if(fat32_node_read_dirent_name(inode, name, &pos))
		return TRUE;
	return FALSE;
}

struct file_operations FAT32_file_ops = {
	.read = FAT32_read,
	.write = FAT32_write,
	.lseek = FAT32_lseek,
	.ioctl = FAT32_ioctl,
	.readdir = FAT32_readdir,
};


long FAT32_create(struct inode * inode, struct dentry * dentry, int mode) {
	return -1;
}


int FAT32_lookup(struct inode * parent_inode, struct dentry * dest_dentry) {
	struct fat32_inode_info * finode = parent_inode->i_private;
	struct fat32_sb_info * fsbi = parent_inode->sb->s_private;
	int ret;
	struct fat32_inode_info * info = kzalloc(sizeof(struct fat32_inode_info), 0);
	if(!info)
		return -1;

	struct fat_dirent_t *dentry = &info->dent;
	if(ret = fat32_node_find_dirent(parent_inode, dest_dentry->name, dentry, info)) {
		kfree(info);
		return ret;
	}


	struct inode * p = kzalloc(sizeof(struct inode), 0);

	p->i_size = dentry->file_size;
	p->i_mode |= (dentry->file_attributes & ATTR_DIRECTORY) ? S_IFDIR : S_IFREG;
	p->i_mode |= (S_IRWXU | S_IRWXG | S_IRWXO);
	
	if(dentry->file_attributes & ATTR_READ_ONLY)
	{
		p->i_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
	}

	p->i_ctime = fatfs_pack_timestamp(dentry->create_date, dentry->create_time);
	p->i_atime = fatfs_pack_timestamp(dentry->laccess_date, (struct fat_time){0});
	p->i_mtime = fatfs_pack_timestamp(dentry->lmodify_date, dentry->lmodify_time);
	
	p->i_dentry = dest_dentry;
	p->sb = parent_inode->sb;
	p->f_ops = &FAT32_file_ops;
	p->inode_ops = &FAT32_inode_ops;
	p->i_private = info;

	dest_dentry->d_inode = p;
	return 0;
}

long FAT32_mkdir(struct inode * parent_inode, char * name, int mode) {
	struct fat32_inode_info * finode = parent_inode->i_private;
	struct fat32_sb_info * fsbi = parent_inode->sb->s_private;
	struct fat_dirent_t dent = {0};
	int ret = -1;

	ret = fat32_node_find_dirent(parent_inode, name, NULL, NULL);
	if(ret != -1) {
		if(!ret)
			return -1;
		else
			return ret;
	}

	u8_t fileattr = ATTR_DIRECTORY;
	if(!(mode & (S_IWUSR | S_IWGRP | S_IWOTH)))
	{
		fileattr |= ATTR_READ_ONLY;
	}
	dent.file_attributes = fileattr;

	struct fat_dirent_t dents[FAT_LONGNAME_MAXSEQ + 1] = {0};

	if((ret = fat32_node_new_dirent(dents, name, &dent)) <= 0)
		return -1;

	//print_hex(dents, ret * sizeof(struct fat_dirent_t));

	ret = fat32_node_write_dirent(parent_inode, dents, ret);
	
	if(ret < 0)
		return ret;
	if(parent_inode != parent_inode->sb->root->d_inode) {
		if(ret){
			parent_inode->sb->sb_ops->write_inode(parent_inode);
		}
	}
	
	return 0;
}

long FAT32_mknod(struct inode * parent_inode, char * name, int mode, dev_t dev) {
	struct fat32_inode_info * finode = parent_inode->i_private;
	struct fat32_sb_info * fsbi = parent_inode->sb->s_private;
	struct fat_dirent_t dent = {0};
	int ret = -1;

	if(!S_ISREG(mode))
		return -1;

	ret = fat32_node_find_dirent(parent_inode, name, NULL, NULL);
	if(ret != -1) {
		if(!ret)
			return -1;
		else
			return ret;
	}

	u8_t fileattr = ATTR_ARCHIVE;
	if(!(mode & (S_IWUSR | S_IWGRP | S_IWOTH)))
	{
		fileattr |= ATTR_READ_ONLY;
	}
	dent.file_attributes = fileattr;

	struct fat_dirent_t dents[FAT_LONGNAME_MAXSEQ + 1] = {0};

	if((ret = fat32_node_new_dirent(dents, name, &dent)) <= 0)
		return -1;

	//print_hex(dents, ret * sizeof(struct fat_dirent_t));

	ret = fat32_node_write_dirent(parent_inode, dents, ret);
	
	if(ret < 0)
		return ret;
	if(parent_inode != parent_inode->sb->root->d_inode) {
		if(ret){
			parent_inode->sb->sb_ops->write_inode(parent_inode);
		}
	}
	
	return 0;
}

long FAT32_rmdir(struct inode * parent_inode, struct dentry * dentry) {
	struct fat32_inode_info * finode = dentry->d_inode->i_private;
	struct fat32_sb_info * fsbi = dentry->d_inode->sb->s_private;
	int ret = -1;

	if(!fat_dir_is_empty(dentry->d_inode))
		return -ENOTEMPTY;
		
	if(!fat32_truncate_clusters(fsbi, finode->first_cluster)) {
		color_printk(RED, BLACK, "FS ERROR:fat32_truncate_clusters failed!\n");
		return -1;
	}
	fat32_del_inode(dentry->d_inode);

	return 0;
}

long FAT32_rename(struct dentry * old_dentry, struct dentry * new_dir_dentry, char *new_name) {
	struct fat32_inode_info * finode = old_dentry->d_inode->i_private;
	struct fat32_sb_info * fsbi = old_dentry->d_inode->sb->s_private;
	struct inode * new_dir_inode = new_dir_dentry->d_inode;
	struct fat_dirent_t dent = {0};
	int ret = -1;

	ret = fat32_node_find_dirent(new_dir_inode, new_name, NULL, NULL);
	if(ret != -1) {
		if(!ret)
			return -1;
		else
			return ret;
	}

	dent = finode->dent;
	fat32_del_inode(old_dentry->d_inode);

	struct fat_dirent_t dents[FAT_LONGNAME_MAXSEQ + 1] = {0};
	if((ret = fat32_node_new_dirent(dents, new_name, &dent)) <= 0)
		return -1;

	ret = fat32_node_write_dirent(new_dir_inode, dents, ret);

	if(ret < 0)
		return ret;

	if(new_dir_inode != new_dir_inode->sb->root->d_inode) {
		if(ret){
			new_dir_inode->sb->sb_ops->write_inode(new_dir_inode);
		}
	}
	return 0;
}

long FAT32_getattr(struct dentry * dentry, unsigned long * attr) {
	return -1;
}

long FAT32_setattr(struct dentry * dentry, unsigned long * attr) {
	return -1;
}

struct inode_operations FAT32_inode_ops = {
	.create = FAT32_create,
	.lookup = FAT32_lookup,
	.mkdir = FAT32_mkdir,
	.mknod = FAT32_mknod,
	.rmdir = FAT32_rmdir,
	.rename = FAT32_rename,
	.getattr = FAT32_getattr,
	.setattr = FAT32_setattr,
};


//// these operation need cache and list
long FAT32_compare(struct dentry * parent_dentry, char * source_filename, char * destination_filename) {
	return -1;
}
long FAT32_hash(struct dentry * dentry, char * filename) {
	return -1;
}
long FAT32_release(struct dentry * dentry) {
	return -1;
}
long FAT32_iput(struct dentry * dentry, struct inode * inode) {
	return -1;
}


struct dentry_operations FAT32_dentry_ops = {
	.compare = FAT32_compare,
	.hash = FAT32_hash,
	.release = FAT32_release,
	.iput = FAT32_iput,
};


void fat32_write_superblock(struct super_block * sb) {}

void fat32_put_superblock(struct super_block * sb) {
	kfree(sb->s_private);
	kfree(sb->root->d_inode->i_private);
	kfree(sb->root->d_inode);
	kfree(sb->root);
	kfree(sb);
}

void fat32_write_inode(struct inode * inode) {
	struct fat_dirent_t dent;
	struct inode * parent_inode = inode->i_dentry->d_parent->d_inode;
	struct fat32_inode_info * finode = inode->i_private;
	struct super_block *sb = inode->sb;
	off_t pos;

	if (inode == sb->root->d_inode) {
		color_printk(RED, BLACK, "FS ERROR:write root inode!\n");
		return;
	}

	pos = finode->dent_off;
	if(fat32_read_clusters(parent_inode, &dent, finode->dent_len, &pos) < 0) {
		color_printk(RED, BLACK, "FS ERROR:read dentry failed!\n");
		return;
	}
	//printf("write inode:read\n");
	//print_hex(&dent, finode->dent_len);
	dent.file_size = inode->i_size;
	dent.first_cluster_lo = finode->first_cluster & 0xffff;
	dent.first_cluster_hi = (dent.first_cluster_hi & 0xf000) | (finode->first_cluster >> 16);
	finode->dent = dent;

	
	//printf("write inode:write\n");
	//print_hex(&dent, finode->dent_len);
	pos = finode->dent_off;
	if(fat32_write_clusters(parent_inode, &dent, finode->dent_len, &pos) < 0) {
		color_printk(RED, BLACK, "FS ERROR:write dentry failed!\n");
		return;
	}
}

void fat32_del_inode(struct inode * inode) {
	struct inode * parent_inode = inode->i_dentry->d_parent->d_inode;
	struct fat32_inode_info * finode = inode->i_private;
	struct super_block *sb = inode->sb;
	struct fat_dirent_t dents[FAT_LONGNAME_MAXSEQ + 1] = {0};
	off_t pos;

	if (inode == sb->root->d_inode) {
		color_printk(RED, BLACK, "FS ERROR:write root inode!\n");
		return;
	}
	if(finode->lfn_len)
		pos = finode->lfn_off;
	else
		pos = finode->dent_off;
	if(fat32_read_clusters(parent_inode, dents, finode->lfn_len + finode->dent_len, &pos) < 0) {
		color_printk(RED, BLACK, "FS ERROR:read dentry failed!\n");
		return;
	}

	for(int i = 0; i < (finode->lfn_len + finode->dent_len) / sizeof(struct fat_dirent_t); i++){
		dents[i].dos_file_name[0] = 0xE5;
	}

	//print_hex(dents, finode->lfn_len + finode->dent_len);
	if(finode->lfn_len)
		pos = finode->lfn_off;
	else
		pos = finode->dent_off;
	if(fat32_write_clusters(parent_inode, dents, finode->lfn_len + finode->dent_len, &pos) < 0) {
		color_printk(RED, BLACK, "FS ERROR:write dentry failed!\n");
		return;
	}
	//TODO:放在这里合适？
	if(d_delete(inode->i_dentry))
		color_printk(RED, BLACK, "FS ERROR:d_delete failed!\n");;
}

struct super_block_operations FAT32_sb_ops = {
	.write_superblock = fat32_write_superblock,
	.put_superblock = fat32_put_superblock,

	.write_inode = fat32_write_inode,
};

struct super_block * fat32_read_superblock(struct block_t * block) {
	////super block
	struct super_block * sbp = kzalloc(sizeof(struct super_block), 0);
	sbp->sb_ops = &FAT32_sb_ops;

	struct fat32_sb_info * fsbi = kzalloc(sizeof(struct fat32_sb_info), 0);
	fsbi->bdev = block;

	struct fat_bootsec_t *fbs = &fsbi->bsec;
	block_read(block, fbs, 0, sizeof(*fbs));

	fsbi->sector_count = block->blksz;
	fsbi->sector_per_cluster = fbs->sectors_per_cluster;
	fsbi->bytes_per_cluster = fbs->sectors_per_cluster * fbs->bytes_per_sector;
	fsbi->bytes_per_sector = fbs->bytes_per_sector;
	fsbi->first_data_sector = fbs->reserved_sector_count + fbs->e32.sectors_per_fat * fbs->number_of_fat;
	fsbi->first_fat_sector = fbs->reserved_sector_count;
	fsbi->sectors_per_fat = fbs->e32.sectors_per_fat;
	fsbi->number_of_fat = fbs->number_of_fat;
	fsbi->fsinfo_sector_infat = fbs->e32.fs_info_sector;
	fsbi->bootsector_bk_infat = fbs->e32.boot_sector_copy;

	////directory entry
	struct dentry *root = d_alloc(NULL, "/");
	root->d_parent = root;
	root->dir_ops = &FAT32_dentry_ops;

	////index node
	struct inode *inode = kzalloc(sizeof(struct inode), 0);
	inode->inode_ops = &FAT32_inode_ops;
	inode->f_ops = &FAT32_file_ops;
	inode->i_size = 0;
	inode->i_mode |= S_IFDIR;
	inode->sb = sbp;
	inode->i_dentry = root;

	////fat32 root inode
	struct fat32_inode_info * finode = kzalloc(sizeof(struct fat32_inode_info), 0);
	finode->first_cluster = fbs->e32.root_directory_cluster;
	finode->fsbi = fsbi;
	finode->dent_len = 0;
	finode->dent_off = 0;
	finode->lfn_len = 0;
	finode->lfn_off = 0;
	finode->create_date = (struct fat_date){0};
	finode->create_time = (struct fat_time){0};
	finode->write_date = (struct fat_date){0};
	finode->write_time = (struct fat_time){0};

	inode->i_private = finode;
	root->d_inode = inode;
	sbp->s_private = fsbi;
	sbp->root = root;

	return sbp;
}


struct filesystem_t fat = {
	.name = "FAT32",
	.fs_flags = 0,
	.read_superblock = fat32_read_superblock,
};

static __init void filesystem_sys_init(void)
{
	register_filesystem(&fat);
}

static __exit void filesystem_sys_exit(void)
{
	unregister_filesystem(&fat);
}

core_initcall(filesystem_sys_init);
core_exitcall(filesystem_sys_exit);

