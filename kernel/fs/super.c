#include <spinlock.h>
#include <vfs.h>
#include <list.h>
#include <printk.h>
#include <errno.h>
#include <memory.h>

static struct super_block *alloc_super(struct file_system_t *type)
{
	struct super_block *s = kzalloc(sizeof(struct super_block),  0);
	if (s) {
		
	}
	return s;
}


struct super_block *sget(struct file_system_t *type)
{
	struct super_block *s = NULL;
	s = alloc_super(type);
	if (s) {
	}
	return s;
}
