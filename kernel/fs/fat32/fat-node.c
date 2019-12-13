#include <printk.h>
#include <task.h>
#include <lib.h>
#include <vfs.h>
#include <errno.h>
#include <stdio.h>
#include <block/block.h>
#include <core/initcall.h>
#include <ctype.h>
#include <core/ktime.h>
#include <clocksource/clocksource.h>
#include "fat32.h"

bool_t fat32_read_next_fat_entry(struct fat32_sb_info *fsbi, u32_t *fat_entry)
{
	u32_t buf[128] = {0};
	if (*fat_entry > 0x0ffffff7 || *fat_entry == 0 || *fat_entry == 1)
		return FALSE;
	if (!block_read(fsbi->bdev, buf, (fsbi->first_fat_sector + (*fat_entry >> 7)) * fsbi->bytes_per_sector, fsbi->bytes_per_sector))
		return FALSE;
	*fat_entry = buf[*fat_entry & 0x7f] & 0x0fffffff;
	if (*fat_entry > 0x0ffffff7 || *fat_entry == 0 || *fat_entry == 1)
		return FALSE;
	return TRUE;
}

bool_t fat32_write_next_fat_entry(struct fat32_sb_info *fsbi, u32_t fat_entry, u32_t value)
{
	unsigned int buf[128] = {0};

	if (!block_read(fsbi->bdev, buf, (fsbi->first_fat_sector + (fat_entry >> 7)) * fsbi->bytes_per_sector, fsbi->bytes_per_sector))
		return FALSE;
	buf[fat_entry & 0x7f] = (buf[fat_entry & 0x7f] & 0xf0000000) | (value & 0x0fffffff);

	for (int i = 0; i < fsbi->number_of_fat; i++)
	{
		if (!block_write(fsbi->bdev, buf, (fsbi->first_fat_sector + fsbi->sectors_per_fat * i + (fat_entry >> 7)) * fsbi->bytes_per_sector, fsbi->bytes_per_sector))
			return FALSE;
	}
	return TRUE;
}

bool_t fat32_read_or_new_next_fat_entry(struct fat32_sb_info *fsbi, unsigned int *pcluster)
{
	u32_t next_cluster = *pcluster;
	u32_t cluster = *pcluster;
	if (!fat32_read_next_fat_entry(fsbi, &next_cluster))
		return FALSE;
	if (next_cluster >= 0x0ffffff8)
	{
		if (!fat32_find_available_cluster(fsbi, &next_cluster))
			return FALSE;
		if (!fat32_write_next_fat_entry(fsbi, cluster, next_cluster))
			return FALSE;
		if (!fat32_write_next_fat_entry(fsbi, next_cluster, 0x0ffffff8))
			return FALSE;
		*pcluster = next_cluster;
	}
	return TRUE;
}

bool_t fat32_truncate_clusters(struct fat32_sb_info *fsbi, u32_t fat_entry)
{
	u32_t current, next = fat_entry;

	while (1)
	{
		current = next;
		if (!fat32_read_next_fat_entry(fsbi, &next))
			return TRUE;
		if (!fat32_write_next_fat_entry(fsbi, current, 0))
			return FALSE;
	}
}

static u8_t fatfs_node_lfn_checksum(const u8_t *name)
{
	int i;
	u8_t sum = 0;

	for (i = 11; i; i--)
	{
		sum = ((sum & 1) << 7) + (sum >> 1) + *name++;
	}

	return sum;
}

long fat32_read_cluster(struct fat32_sb_info *fsbi, u32_t cluster, u8_t *buf)
{
	u32_t sector = fsbi->first_data_sector + (cluster - 2) * fsbi->sector_per_cluster;
	if (!block_read(fsbi->bdev, buf, sector * fsbi->bytes_per_sector, fsbi->bytes_per_cluster))
	{
		LOGE("[FAT32] fat32_read_cluster ERROR!!!!!!!!!!");
		return -EIO;
	}
	return 0;
}

long fat32_write_cluster(struct fat32_sb_info *fsbi, u32_t cluster, u8_t *buf)
{
	u32_t sector = fsbi->first_data_sector + (cluster - 2) * fsbi->sector_per_cluster;
	if (!block_write(fsbi->bdev, buf, sector * fsbi->bytes_per_sector, fsbi->bytes_per_cluster))
	{
		LOGE("[FAT32] fat32_write_cluster ERROR!!!!!!!!!!");
		return -EIO;
	}
	return 0;
}

static long _fat32_node_read(struct fat32_sb_info *fsbi, u32_t cluster, u8_t *to, u8_t *buf, size_t count, off_t off)
{
	u8_t *from = buf;
	if (fat32_read_cluster(fsbi, cluster, buf))
		return -EIO;

	if ((unsigned long)buf < CONFIG_TASK_SIZE)
		copy_to_user(to, from + off, count);
	else
		memcpy(to, from + off, count);
	return 0;
}

static long _fat32_node_write(struct fat32_sb_info *fsbi, u32_t cluster, u8_t *from, u8_t *buf, size_t count, off_t off)
{
	u8_t *to = buf;
	//TODO：新的cluster可以节省一次读写
	if (fat32_read_cluster(fsbi, cluster, buf))
		return -EIO;

	if ((unsigned long)buf < CONFIG_TASK_SIZE)
		copy_to_user(to + off, from, count);
	else
		memcpy(to + off, from, count);

	if (fat32_write_cluster(fsbi, cluster, buf))
		return -EIO;
	return 0;
}

u32_t fat32_read_clusters(struct inode *inode, u8_t *buf, size_t count, off_t *pos)
{
	struct fat32_inode_info *finode = dynamic_cast(fat32_inode_info)(inode);
	struct fat32_sb_info *fsbi = finode->fsbi;

	off_t off = *pos % fsbi->bytes_per_cluster;
	u32_t ret = 0;
	u32_t r = 0;

	u32_t cluster = finode->first_cluster;
	if (!cluster)
		return -EFAULT;

	for (int i = *pos / fsbi->bytes_per_cluster; i > 0; i--)
	{
		if (!fat32_read_next_fat_entry(fsbi, &cluster))
			return -ENOSPC;
	}

	char *cached_data = (char *)kmalloc(fsbi->bytes_per_cluster, 0);

	if (!cached_data)
		return -ENOSPC;

	do
	{
		memset(cached_data, 0, fsbi->bytes_per_cluster);
		int len = count <= fsbi->bytes_per_cluster - off ? count : fsbi->bytes_per_cluster - off;
		if (ret = _fat32_node_read(fsbi, cluster, buf, cached_data, len, off))
			break;
		count -= len;
		buf += len;
		off -= off;
		r += len;
		*pos += len;
	} while (count && (fat32_read_next_fat_entry(fsbi, &cluster)));

	kfree(cached_data);
	if (!count)
		ret = r;
	return ret;
}

u32_t fat32_write_clusters(struct inode *inode, u8_t *buf, size_t count, off_t *pos)
{
	struct fat32_inode_info *finode = dynamic_cast(fat32_inode_info)(inode);
	struct fat32_sb_info *fsbi = finode->fsbi;

	u32_t cluster = finode->first_cluster;
	off_t off = *pos % fsbi->bytes_per_cluster;
	u32_t ret = 0;
	u32_t w = 0;

	if (!cluster)
	{
		if (!fat32_find_available_cluster(fsbi, &cluster))
			return -ENOSPC;
		finode->first_cluster = cluster;

		if (!fat32_write_next_fat_entry(fsbi, cluster, 0x0ffffff8))
			return -EFAULT;
	}
	else
	{
		for (int i = *pos / fsbi->bytes_per_cluster; i > 0; i--)
		{
			if (!fat32_read_next_fat_entry(fsbi, &cluster))
				return -ENOSPC;
		}
	}

	char *cached_data = (char *)kmalloc(fsbi->bytes_per_cluster, 0);

	do
	{
		memset(cached_data, 0, fsbi->bytes_per_cluster);
		int len = count <= fsbi->bytes_per_cluster - off ? count : fsbi->bytes_per_cluster - off;
		if (ret = _fat32_node_write(fsbi, cluster, buf, cached_data, len, off))
			break;

		count -= len;
		buf += len;
		off -= off;
		w += len;
		*pos += len;
	} while (count && fat32_read_or_new_next_fat_entry(fsbi, &cluster));

	kfree(cached_data);
	if (!count)
		ret = w;
	return ret;
}

u32_t fat32_node_read(struct inode *inode, u8_t *buf, size_t count, off_t *pos)
{
	struct fat32_inode_info *finode = dynamic_cast(fat32_inode_info)(inode);
	struct fat32_sb_info *fsbi = finode->fsbi;
	off_t off = *pos % fsbi->bytes_per_cluster;
	u32_t filesize = inode->i_size;

	if (filesize <= (u32_t)*pos)
		return 0;

	if (*pos + count > filesize)
		count = filesize - *pos;

	return fat32_read_clusters(inode, buf, count, pos);
}

u32_t fat32_node_write(struct inode *inode, u8_t *buf, size_t count, off_t *pos)
{
	return fat32_write_clusters(inode, buf, count, pos);
}

int fat32_node_find_dirent(struct inode *inode, char *filename, struct fat_dirent_t *fat_dentry, struct fat32_inode_info *info)
{
	struct fat32_inode_info *finode = dynamic_cast(fat32_inode_info)(inode);
	struct fat32_sb_info *fsbi = finode->fsbi;
	u32_t lfn_len = 0, lfn_off = 0;
	off_t pos = 0;
	int ret = -1;
	char name[VFS_MAX_NAME] = {0};

	u32_t cluster = finode->first_cluster;
	if (!cluster)
		return -1;

	char *cached_data = kmalloc(fsbi->bytes_per_cluster, 0);
	if (!cached_data)
		return -ENOSPC;

	do
	{
		if (ret = fat32_read_cluster(fsbi, cluster, cached_data))
			return ret;

		struct fat_dirent_t *dent = (struct fat_dirent_t *)cached_data;

		int len = fsbi->bytes_per_cluster;

		for (int i = 0; i < len; i += 32, dent++)
		{
			if (dent->dos_file_name[0] == 0xe5 || dent->dos_file_name[0] == 0x00 || dent->dos_file_name[0] == 0x05)
				continue;

			if (dent->file_attributes == ATTR_LONG_NAME)
			{
				struct fat_longname_t *lfn = dent;
				int seqno = lfn->seqno;
				if (FAT_LONGNAME_LASTSEQ(lfn->seqno))
				{
					seqno = FAT_LONGNAME_SEQNO(lfn->seqno);
					lfn_len = seqno * sizeof(struct fat_longname_t);
					lfn_off = pos + i;
				}

				if ((seqno < FAT_LONGNAME_MINSEQ) || (FAT_LONGNAME_MAXSEQ < seqno))
				{
					continue;
				}

				int len = (seqno - 1) * 13;

				name[len + 0] = (char)(lfn->name_utf16_1[0]);
				name[len + 1] = (char)(lfn->name_utf16_1[1]);
				name[len + 2] = (char)(lfn->name_utf16_1[2]);
				name[len + 3] = (char)(lfn->name_utf16_1[3]);
				name[len + 4] = (char)(lfn->name_utf16_1[4]);
				name[len + 5] = (char)(lfn->name_utf16_2[0]);
				name[len + 6] = (char)(lfn->name_utf16_2[1]);
				name[len + 7] = (char)(lfn->name_utf16_2[2]);
				name[len + 8] = (char)(lfn->name_utf16_2[3]);
				name[len + 9] = (char)(lfn->name_utf16_2[4]);
				name[len + 10] = (char)(lfn->name_utf16_2[5]);
				name[len + 11] = (char)(lfn->name_utf16_3[0]);
				name[len + 12] = (char)(lfn->name_utf16_3[1]);

				if (FAT_LONGNAME_LASTSEQ(lfn->seqno))
				{
				}
				continue;
			}

			if (dent->file_attributes & ATTR_VOLUME_ID)
			{
				continue;
			}

			if (!strlen(name))
			{
				lfn_len = 0;
				lfn_off = 0;
				int j = 0;
				for (int x = 0; x < 8 && dent->dos_file_name[x] != ' '; x++)
				{
					if (dent->DIR_NTRes & LOWERCASE_BASE)
						name[j++] = tolower(dent->dos_file_name[x]);
					else
						name[j++] = dent->dos_file_name[x];
				}

				if (!(dent->file_attributes & ATTR_DIRECTORY))
				{
					if (dent->dos_file_name[8] != ' ')
					{
						name[j++] = '.';

						for (int x = 0; x < 3 && dent->dos_file_name[8 + x] != ' '; x++)
						{
							if (dent->DIR_NTRes & LOWERCASE_EXT)
								name[j++] = tolower(dent->dos_file_name[8 + x]);
							else
								name[j++] = dent->dos_file_name[8 + x];
						}
					}
				}
			}

			//printf("[%s]\n", name);
			if (!strncmp(name, filename, VFS_MAX_NAME))
			{
				if (fat_dentry)
					*fat_dentry = *dent;

				if (info)
				{
					info->first_cluster = (dent->first_cluster_hi << 16 | dent->first_cluster_lo) & 0x0fffffff;
					info->fsbi = fsbi;
					info->dent_off = pos + i;
					info->dent_len = sizeof(struct fat_dirent_t);
					info->lfn_off = lfn_off;
					info->lfn_len = lfn_len;
					info->create_date = dent->create_date;
					info->create_time = dent->create_time;
					info->write_date = dent->lmodify_date;
					info->write_time = dent->lmodify_time;
				}

				//printf("[%s %d - %d]\n", name, (lfn_len == 0 ? info->dent_off / 32 : lfn_off / 32), info->dent_off / 32);
				kfree(cached_data);
				return 0;
			}
			memset(name, 0, sizeof(name));
		}
		pos += len;
	} while (fat32_read_next_fat_entry(fsbi, &cluster));

	kfree(cached_data);
	return -1;
}

int fat32_node_read_dirent_name(struct inode *inode, char *buf, off_t *pos)
{
	struct fat32_inode_info *finode = dynamic_cast(fat32_inode_info)(inode);
	struct fat32_sb_info *fsbi = finode->fsbi;
	off_t off = *pos % fsbi->bytes_per_cluster;
	int ret = -1;
	char name[VFS_MAX_NAME] = {0};

	u32_t cluster = finode->first_cluster;
	if (!cluster)
		return -EFAULT;

	char *cached_data = kmalloc(fsbi->bytes_per_cluster, 0);
	if (!cached_data)
		return -ENOSPC;

	for (int i = *pos / fsbi->bytes_per_cluster; i > 0; i--)
	{
		if (!fat32_read_next_fat_entry(fsbi, &cluster))
			return -ENOSPC;
	}

	do
	{
		if (ret = fat32_read_cluster(fsbi, cluster, cached_data))
			return ret;

		struct fat_dirent_t *dent = (struct fat_dirent_t *)(cached_data + off);

		int len = fsbi->bytes_per_cluster - off;

		for (int i = 0; i < len; i += 32, dent++)
		{
			if (dent->dos_file_name[0] == 0xe5 || dent->dos_file_name[0] == 0x00 || dent->dos_file_name[0] == 0x05)
				continue;

			if (dent->file_attributes == ATTR_LONG_NAME)
			{
				struct fat_longname_t *lfn = dent;
				int seqno = lfn->seqno;
				if (FAT_LONGNAME_LASTSEQ(lfn->seqno))
				{
					seqno = FAT_LONGNAME_SEQNO(lfn->seqno);
				}

				if ((seqno < FAT_LONGNAME_MINSEQ) || (FAT_LONGNAME_MAXSEQ < seqno))
				{
					continue;
				}

				int len = (seqno - 1) * 13;

				name[len + 0] = (char)(lfn->name_utf16_1[0]);
				name[len + 1] = (char)(lfn->name_utf16_1[1]);
				name[len + 2] = (char)(lfn->name_utf16_1[2]);
				name[len + 3] = (char)(lfn->name_utf16_1[3]);
				name[len + 4] = (char)(lfn->name_utf16_1[4]);
				name[len + 5] = (char)(lfn->name_utf16_2[0]);
				name[len + 6] = (char)(lfn->name_utf16_2[1]);
				name[len + 7] = (char)(lfn->name_utf16_2[2]);
				name[len + 8] = (char)(lfn->name_utf16_2[3]);
				name[len + 9] = (char)(lfn->name_utf16_2[4]);
				name[len + 10] = (char)(lfn->name_utf16_2[5]);
				name[len + 11] = (char)(lfn->name_utf16_3[0]);
				name[len + 12] = (char)(lfn->name_utf16_3[1]);

				if (FAT_LONGNAME_LASTSEQ(lfn->seqno))
				{
				}
				continue;
			}

			if (dent->file_attributes & ATTR_VOLUME_ID)
			{
				continue;
			}

			if (!strlen(name))
			{
				int j = 0;
				for (int x = 0; x < 8 && dent->dos_file_name[x] != ' '; x++)
				{
					if (dent->DIR_NTRes & LOWERCASE_BASE)
						name[j++] = tolower(dent->dos_file_name[x]);
					else
						name[j++] = dent->dos_file_name[x];
				}

				if (!(dent->file_attributes & ATTR_DIRECTORY))
				{
					if (dent->dos_file_name[8] != ' ')
					{
						name[j++] = '.';

						for (int x = 0; x < 3 && dent->dos_file_name[8 + x] != ' '; x++)
						{
							if (dent->DIR_NTRes & LOWERCASE_EXT)
								name[j++] = tolower(dent->dos_file_name[8 + x]);
							else
								name[j++] = dent->dos_file_name[8 + x];
						}
					}
				}
			}

			strncpy(buf, name, VFS_MAX_NAME);
			*pos += i + 32;
			return 0;
		}

		*pos += len;
		off -= off;
	} while (fat32_read_next_fat_entry(fsbi, &cluster));

	kfree(cached_data);
	return -1;
}

int fat32_node_new_dirent(struct fat_dirent_t *dents, char *name, struct fat_dirent_t *ndent)
{
	struct fat_dirent_t *dent = NULL;
	u32_t len, off, dent_cnt;
	u8_t dcsum, check[11];
	//struct fat_dirent_t dents[FAT_LONGNAME_MAXSEQ + 1] = {0};
	int ret = -1;

	if (!dents || !name || !ndent)
		return -1;
	len = strlen(name) + 1;
	dent_cnt = (len + 12) / 13 + 1;

	if (dent_cnt > FAT_LONGNAME_MAXSEQ + 1)
		return -1;

	dent = &dents[dent_cnt - 1];

	/* Prepare final directory entry */
	snprintf((char *)check, sizeof(check), "%08x", (u32_t)ktime_to_ns(ktime_get()));
	memcpy(dent, ndent, sizeof(struct fat_dirent_t));

	dent->dos_file_name[0] = ' ';
	dent->dos_file_name[1] = '\0';
	dent->dos_file_name[2] = check[0]; /* Random byte */
	dent->dos_file_name[3] = check[1]; /* Random byte */
	dent->dos_file_name[4] = check[2]; /* Random byte */
	dent->dos_file_name[5] = check[3]; /* Random byte */
	dent->dos_file_name[6] = check[4]; /* Random byte */
	dent->dos_file_name[7] = check[5]; /* Random byte */
	dent->dos_file_name[8] = '/';
	dent->dos_file_name[9] = check[6];  /* Random byte */
	dent->dos_file_name[10] = check[7]; /* Random byte */

	/* Compute checksum of final directory entry */
	dcsum = fatfs_node_lfn_checksum(dent->dos_file_name);

	off = 0;
	for (int i = dent_cnt - 2; i >= 0; i--)
	{
		struct fat_longname_t *longdent = &dents[i];
		longdent->seqno = FAT_LONGNAME_SEQNO((dent_cnt - 1) - i);
		if (i == 0)
			longdent->seqno |= FAT_LONGNAME_LASTSEQ_MASK;
		longdent->file_attributes = FAT_LONGNAME_ATTRIBUTE;
		longdent->type = 0;
		longdent->checksum = dcsum;
		longdent->first_cluster = 0;

		for (int j = 0; j < 13; j++)
		{
			u16_t temp;

			if (off >= len)
				temp = 0xffff;
			else
				temp = name[off++];

			if (j < 5)
				longdent->name_utf16_1[j] = temp;
			else if (j < 11)
				longdent->name_utf16_2[j - 5] = temp;
			else
				longdent->name_utf16_3[j - 11] = temp;
		}
	}
	return dent_cnt;
}

int fat32_node_write_dirent(struct inode *inode, struct fat_dirent_t *dents, int dent_cnt)
{
	int ret = -1;
	struct fat32_inode_info *finode = dynamic_cast(fat32_inode_info)(inode);
	struct fat32_sb_info *fsbi = finode->fsbi;

	u32_t cluster = finode->first_cluster;
	u32_t cnt = 0;
	off_t off = 0;
	u32_t lfn_off = 0;
	bool_t new = FALSE;

	if (!dents || !dent_cnt)
		return -1;

	if (!cluster)
	{
		if (!fat32_find_available_cluster(fsbi, &cluster))
			return -ENOSPC;
		finode->first_cluster = cluster;

		if (!fat32_write_next_fat_entry(fsbi, cluster, 0x0ffffff8))
			return -EFAULT;
		new = TRUE;
	}

	char *cached_data = (char *)kmalloc(fsbi->bytes_per_cluster, 0);

	do
	{
		memset(cached_data, 0, fsbi->bytes_per_cluster);

		if (new)
		{
			if (ret = fat32_write_cluster(fsbi, cluster, cached_data))
				break;
		}
		else
		{
			if (ret = fat32_read_cluster(fsbi, cluster, cached_data))
				break;
		}

		struct fat_dirent_t *dent = cached_data;

		for (int i = 0; i < fsbi->bytes_per_cluster; i += 32, dent++, off += 32)
		{
			if (dent->dos_file_name[0] == 0xe5 || dent->dos_file_name[0] == 0x00 || dent->dos_file_name[0] == 0x05)
			{
				if (!cnt)
					lfn_off = off;
				cnt++;
				if (cnt >= dent_cnt)
				{
					kfree(cached_data);
					//printf("[%d --- %d]\n", lfn_off / 32, off / 32);
					if (!fat32_write_clusters(inode, dents, dent_cnt * sizeof(struct fat_dirent_t), &lfn_off) < 0)
						return -1;
					else
						return new;
				}
			}
			else
			{
				cnt = 0;
			}
		}

		if (!fat32_read_next_fat_entry(fsbi, &cluster))
		{
			if (!fat32_read_or_new_next_fat_entry(fsbi, &cluster))
				break;
			new = TRUE;
		}
	} while (1);

	kfree(cached_data);

	return -1;
}
