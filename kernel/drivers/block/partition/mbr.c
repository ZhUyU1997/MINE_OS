/*
 * driver/block/partition/mbr.c
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

#include <block/partition.h>
#include <core/initcall.h>
#include <malloc.h>

struct mbr_entry_t
{
	uint8_t flag;
	uint8_t start_head;
	uint8_t start_sector;
	uint8_t start_cylinder;
	uint8_t type;
	uint8_t end_head;
	uint8_t end_sector;
	uint8_t end_cylinder;
	uint8_t start[4];
	uint8_t length[4];
} __attribute__ ((packed));

struct mbr_header_t
{
	uint8_t code[446];
	struct mbr_entry_t entry[4];
	uint8_t signature[2];
} __attribute__ ((packed));

static bool_t is_extended(uint8_t type)
{
	if((type == 0x5) || (type == 0xf) || (type == 0x85))
		return TRUE;
	return FALSE;
}

static bool_t mbr_map(struct disk_t * disk)
{
	struct mbr_header_t mbr;
	struct partition_t * part;
	int i;

	if(!disk || !disk->name)
		return FALSE;

	if(!disk->size || !disk->count)
		return FALSE;

	if(disk_read(disk, (uint8_t *)(&mbr), 0, sizeof(struct mbr_header_t)) != sizeof(struct mbr_header_t))
		return FALSE;

	if((mbr.signature[0] != 0x55) || mbr.signature[1] != 0xaa)
		return FALSE;

	if((mbr.entry[0].type == 0xee) || (mbr.entry[1].type == 0xee) || (mbr.entry[2].type == 0xee) || (mbr.entry[3].type == 0xee))
		return FALSE;

	for(i = 0; i < 4; i++)
	{
		if((mbr.entry[i].type != 0) && (!is_extended(mbr.entry[i].type)))
		{
			part = malloc(sizeof(struct partition_t));
			if(!part)
				return FALSE;

			strlcpy(part->name, "", sizeof(part->name));
			part->from = ((mbr.entry[i].start[3] << 24) | (mbr.entry[i].start[2] << 16) | (mbr.entry[i].start[1] << 8) | (mbr.entry[i].start[0] << 0));
			part->to = part->from + ((mbr.entry[i].length[3] << 24) | (mbr.entry[i].length[2] << 16) | (mbr.entry[i].length[1] << 8) | (mbr.entry[i].length[0] << 0)) - 1;
			part->size = disk->size;
			list_add_tail(&part->entry, &(disk->part.entry));
		}
	}
	return TRUE;
}

static struct partition_map_t mbr = {
	.name	= "mbr",
	.map	= mbr_map,
};

static __init void partition_map_mbr_init(void)
{
	register_partition_map(&mbr);
}

static __exit void partition_map_mbr_exit(void)
{
	unregister_partition_map(&mbr);
}

core_initcall(partition_map_mbr_init);
core_exitcall(partition_map_mbr_exit);
