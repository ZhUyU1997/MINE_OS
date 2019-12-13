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

#ifndef __FAT32_H__
#define __FAT32_H__

#include <types.h>
#include <class.h>
/*
 * Extended boot sector information for FAT12/FAT16
 */
struct fat_bootsec_ext16_t
{
	u8_t drive_number;
	u8_t reserved;
	u8_t extended_signature;
	u32_t serial_number;
	u8_t volume_label[11];
	u8_t fs_type[8];
	u8_t boot_code[448];
	u16_t boot_sector_signature;
} __attribute__((packed));

/*
 * Extended boot sector information for FAT32
 */
struct fat_bootsec_ext32_t
{
	u32_t sectors_per_fat;
	u16_t fat_flags;
	u16_t version;
	u32_t root_directory_cluster;
	u16_t fs_info_sector;
	u16_t boot_sector_copy;
	u8_t reserved1[12];
	u8_t drive_number;
	u8_t reserved2;
	u8_t extended_signature;
	u32_t serial_number;
	u8_t volume_label[11];
	u8_t fs_type[8];
	u8_t boot_code[420];
	u16_t boot_sector_signature;
} __attribute__((packed));

struct fat_bootsec_t
{
	u8_t jump[3];
	u8_t oem_name[8];
	u16_t bytes_per_sector;
	u8_t sectors_per_cluster;
	u16_t reserved_sector_count;
	u8_t number_of_fat;
	u16_t root_entry_count;
	u16_t total_sectors_16;
	u8_t media_type;
	u16_t sectors_per_fat;
	u16_t sectors_per_track;
	u16_t number_of_heads;
	u32_t hidden_sector_count;
	u32_t total_sectors_32;

	union {
		struct fat_bootsec_ext16_t e16;
		struct fat_bootsec_ext32_t e32;
	};
} __attribute__((packed));

struct fat32_fs_info
{
	unsigned int FSI_LeadSig;
	unsigned char FSI_Reserved1[480];
	unsigned int FSI_StrucSig;
	unsigned int FSI_Free_Count;
	unsigned int FSI_Nxt_Free;
	unsigned char FSI_Reserved2[12];
	unsigned int FSI_TrailSig;
} __attribute__((packed));

#define ATTR_READ_ONLY (1 << 0)
#define ATTR_HIDDEN (1 << 1)
#define ATTR_SYSTEM (1 << 2)
#define ATTR_VOLUME_ID (1 << 3)
#define ATTR_DIRECTORY (1 << 4)
#define ATTR_ARCHIVE (1 << 5)
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

#define FAT_LONGNAME_ATTRIBUTE (0x0F)
#define FAT_LONGNAME_LASTSEQ_MASK (0x40)
#define FAT_LONGNAME_SEQNO(s) ((s) & ~0x40)
#define FAT_LONGNAME_LASTSEQ(s) ((s)&0x40)
#define FAT_LONGNAME_MINSEQ (1)
#define FAT_LONGNAME_MAXSEQ (256 / 13)

struct fat_date
{
	u16_t day : 5;
	u16_t month : 4;
	u16_t year : 7;
} __attribute__((packed));

struct fat_time
{
	u16_t seconds : 5;
	u16_t minutes : 6;
	u16_t hours : 5;
} __attribute__((packed));

struct fat_dirent_t
{
	u8_t dos_file_name[11];
	u8_t file_attributes;
	u8_t DIR_NTRes;
	//EXT|BASE => 8(BASE).3(EXT)
	//BASE:LowerCase(8),UpperCase(0)
	//EXT:LowerCase(16),UpperCase(0)
	u8_t create_time_millisecs;
	struct fat_time create_time;
	struct fat_date create_date;
	struct fat_date laccess_date;
	u16_t first_cluster_hi;
	struct fat_time lmodify_time;
	struct fat_date lmodify_date;
	u16_t first_cluster_lo;
	u32_t file_size;
} __attribute__((packed));

#define LOWERCASE_BASE (8)
#define LOWERCASE_EXT (16)

struct fat_longname_t
{
	u8_t seqno;
	u16_t name_utf16_1[5];
	u8_t file_attributes;
	u8_t type;
	u8_t checksum;
	u16_t name_utf16_2[6];
	u16_t first_cluster;
	u16_t name_utf16_3[2];
} __attribute__((packed));

/////////////FAT32 for VFS

CLASS_DEF(fat32_sb_info)
{
	/* FAT boot sector */
	struct fat_bootsec_t bsec;

	/* Underlying block device */
	struct block_t *bdev;

	u32_t sector_count;

	u32_t sector_per_cluster;
	u32_t bytes_per_cluster;
	u32_t bytes_per_sector;

	u32_t first_data_sector;
	u32_t first_fat_sector;
	u32_t sectors_per_fat;
	u32_t number_of_fat;

	u32_t fsinfo_sector_infat;
	u32_t bootsector_bk_infat;

	struct fat32_fs_info *fat_fsinfo;

	void (*init)(fat32_sb_info * self, struct block_t * block, struct fat_bootsec_t * fbs);
};

struct fat32_pos
{
	u32_t cluster;
	u32_t off;
	u32_t len;
};

CLASS_DEF(fat32_inode_info)
{
	u32_t first_cluster;

	u32_t dent_off; //dentry struct offset in cluster
	u32_t dent_len;

	u32_t lfn_off;
	u32_t lfn_len;

	struct fat_dirent_t dent;

	struct fat_date create_date;
	struct fat_time create_time;

	struct fat_date write_date;
	struct fat_time write_time;

	struct fat32_sb_info *fsbi;

	void (*init)(fat32_inode_info * self, fat32_sb_info * fsbi, u32_t first);
};

long fat32_read_cluster(struct fat32_sb_info *fsbi, u32_t cluster, u8_t *buf);
long fat32_write_cluster(struct fat32_sb_info *fsbi, u32_t cluster, u8_t *buf);

u32_t fat32_read_clusters(struct inode *inode, u8_t *buf, size_t count, off_t *pos);
u32_t fat32_write_clusters(struct inode *inode, u8_t *buf, size_t count, off_t *pos);

u32_t fat32_node_read(struct inode *inode, u8_t *buf, size_t count, off_t *pos);
u32_t fat32_node_write(struct inode *inode, u8_t *buf, size_t count, off_t *pos);

bool_t fat32_read_next_fat_entry(struct fat32_sb_info *fsbi, u32_t *fat_entry);
bool_t fat32_write_next_fat_entry(struct fat32_sb_info *fsbi, u32_t fat_entry, u32_t value);
bool_t fat32_find_available_cluster(struct fat32_sb_info *fsbi, u32_t *fat_entry);
bool_t fat32_truncate_clusters(struct fat32_sb_info *fsbi, u32_t fat_entry);

int fat32_node_find_dirent(struct inode *parent_inode, char *filename, struct fat_dirent_t *fat_dentry, struct fat32_inode_info *info);
int fat32_node_read_dirent_name(struct inode *inode, char *buf, off_t *pos);
int fat32_node_write_dirent(struct inode *inode, struct fat_dirent_t *dents, int dent_cnt);

extern struct inode_operations FAT32_inode_ops;
extern struct file_operations FAT32_file_ops;
extern struct dentry_operations FAT32_dentry_ops;
extern struct super_block_operations FAT32_sb_ops;

#endif
