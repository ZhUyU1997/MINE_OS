#include <core/machine.h>
#include <class.h>
#include <assert.h>

struct class_head_info {
	unsigned int type;
	unsigned int size;
	unsigned int parent;
	unsigned int child;
};

extern unsigned int __class_type_start[];
extern unsigned int __class_type_end[];

extern struct class_table_info __class_table_start[];
extern struct class_table_info __class_table_end[];

CLASS(object_t) {};

char * get_class_name(void *obj) {
	if(obj) {
		struct class_head_info * info = obj - sizeof(struct class_head_info);
		return __class_table_start[info->type].name;
	}
	return "unknown";
}

unsigned int get_class_type(void *obj)
{
	if(obj) {
		struct class_head_info * info = obj - sizeof(struct class_head_info);
		return __class_table_start[info->type].type;
	}
	return "unknown";
}

char * __class_name(unsigned int type)
{
	return __class_table_start[type].name;
}

void show_class_structure(void *obj) {
	if(obj) {
		struct class_head_info * info = obj - sizeof(struct class_head_info);
		if(info->type == class_type(object_t)) {
			printf("%s\n",__class_table_start[info->type].name);
			return;
		} else
			printf("%s->",__class_table_start[info->type].name);
		//printf("(%llX %d %llX)",info, table[table[info->type].parent].size, (char *)info - table[table[info->type].parent].size);
		show_class_structure((char *)info - __class_table_start[__class_table_start[info->type].parent_type].size);
	} else
		printf("unknown") ;
}

void object_init(unsigned int type, unsigned int child, char *data) {
	char *obj = data - __class_table_start[type].size;
	struct class_head_info *info = obj - sizeof(struct class_head_info);
	info->type = type;
	info->child = child;
	//printf("type: %d data:%X size:%d info:%X\n",type,data,table[type].size,info);
	if(type == class_type(object_t)) {
		//printf("%d",type);
		return;
	} else {
		//printf("%d",type);
	}
	object_init(__class_table_start[type].parent_type, type, info);
	//printf("memcpy %llX %llX %d\n",obj + sizeof(struct class_info),table[type].init_obj, table[type].size);
	memcpy(obj, __class_table_start[type].init_obj, __class_table_start[type].size);
}
char * new_alloc(unsigned int type) {
	char *obj = calloc(1, __class_table_start[type].full_size);
	//printf("fullsize:%d obj=%X\n",table[type].full_size,obj);
	if(obj) {
		object_init(type, type, obj + __class_table_start[type].full_size);
		return obj + __class_table_start[type].full_size - __class_table_start[type].size;
	}
	return NULL;
}

void delete_free(char *obj)
{
	assert(obj);
	unsigned int type = get_class_type(obj);
	char *base = obj - __class_table_start[type].full_size + __class_table_start[type].size;
	free(base);
}

void * class_cast(int type, char *obj) {
	//printf("type = %d\n",type);

	for(;;) {
		struct class_head_info * info = obj - sizeof(struct class_head_info);
		if(info->type == type) {
			return obj;
		} else if(info->type == info->child) {
			break;
		}
		obj = (char *)obj + __class_table_start[info->type].size + sizeof(struct class_head_info);
	}

	for(;;) {
		struct class_head_info * info = obj - sizeof(struct class_head_info);
		if(info->type == type) {
			return obj;
		} else if(info->type == class_type(object_t)) {
			return NULL;
		}
		obj = (char *)info - __class_table_start[__class_table_start[info->type].parent_type].size;
	}
}

static unsigned int set_class_table_info(struct class_table_info *info){
	unsigned int size = 0;
	if(info->type != object_t_class_type)
		size = set_class_table_info(&__class_table_start[info->parent_type]);
	info->full_size = size + sizeof(struct class_head_info) + info->size;
	return info->full_size;
}

void do_init_class()
{

	struct class_table_info *info;
	unsigned int type;

	info =  &(*__class_table_start);
	type = 0;
	while(info < &(*__class_table_end))
	{
		info->type = type;
		info->pointer_type[0] = type;
		type++;
		info++;
	}

	info =  &(*__class_table_start);
	while(info < &(*__class_table_end))
	{
		info->parent_type = info->pointer_parent_type[0];
		info++;
	}

	info =  &(*__class_table_start);
	while(info < &(*__class_table_end))
	{
		set_class_table_info(info);
		printf("class: %s type: %d parent_type: %d size: %d full_size: %d\n", info->name, info->type, info->parent_type, info->size, info->full_size);
		info++;
	}
}