#include <spinlock.h>
#include <vfs.h>
#include <list.h>
#include <printk.h>
#include <errno.h>
#include <memory.h>

struct dentry *d_alloc(struct dentry * parent, const char *name)
{
	struct dentry *path = (struct dentry *)kzalloc(sizeof(struct dentry), 0);
	if(!path) return NULL;

	memset(path, 0, sizeof(struct dentry));
	
	init_list_head(&path->child_node);
	init_list_head(&path->subdirs_list);

	int len = strlcpy(path->name, name, sizeof(path->name));
	if(len >= sizeof(path->name)) {
		kfree(path);
		return NULL;
	}
	
	path->d_parent = parent;
	return path;
}

struct dentry * d_lookup(struct dentry * parent, char *name){
	struct list_head *i;

	if (!strncmp(name, ".", VFS_MAX_NAME)) {
		return parent;
	} else if(!strncmp(name, "..", VFS_MAX_NAME)) {
		return parent->d_parent;
	}

	list_for_each(i,&parent->subdirs_list){
		struct dentry *temp = container_of(i,struct dentry,child_node);
		if(!strncmp(name, temp->name, sizeof(temp->name))){
			return temp;
		}
	}
	return NULL;
}


int d_delete(struct dentry * dent){
	list_del(&dent->child_node);

	if(dent->d_inode->i_private && !kfree(dent->d_inode->i_private)) return -1;
	if(dent->d_inode && !kfree(dent->d_inode)) return -1;
	if(dent && !kfree(dent)) return -1;
	return 0;
}
