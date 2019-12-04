#ifndef __CLASS_H__
#define __CLASS_H__

#ifdef __cplusplus
extern "C" {
#endif

struct class_table_info {
	char *name;
	unsigned int type;
	unsigned int size;
	unsigned int full_size;
	unsigned int parent_type;
	void *init_obj;
	unsigned int *pointer_type;
	unsigned int *pointer_parent_type;
};

char * get_class_name(void *obj);
unsigned int get_class_type(void *obj);
char * __class_name(unsigned int type);
#define class_name(type) __class_name(type##_class_type)
#define class_type(type) type##_class_type

void show_class_structure(void *obj);
void object_init(unsigned int type, unsigned int child, char *data);
char * new_alloc(unsigned int type);
void delete_free(char *obj);
void * class_cast(int type, char *obj);

#define PP_VARIADIC_SIZE(...) PP_VARIADIC_SIZE_I(x, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define PP_VARIADIC_SIZE_I(x, e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, size, ...) size

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) a##b

#define __CLASS(type_name, parent_type_name) \
	unsigned int type_name##_class_type __attribute__((__used__, __section__(".class.type"))) = ~0; \
	extern unsigned int parent_type_name##_class_type; \
	static type_name type_name##_class_init; \
	static struct class_table_info type_name##class_table_info __attribute__((__used__, __section__(".class.table"))) = { \
		.name  = #type_name, \
		.pointer_type = &type_name##_class_type, \
		.pointer_parent_type = &parent_type_name##_class_type, \
		.size = sizeof(type_name), \
		.init_obj = &type_name##_class_init, \
	}; \
	static type_name type_name##_class_init =  

#define __CLASS0(type_name) __CLASS(type_name, object_t)
#define __CLASS1(type_name, parent_type_name) __CLASS(type_name, parent_type_name)
#define CLASS(type_name, args...) PP_CAT(__CLASS,PP_VARIADIC_SIZE(args))(type_name, ##args)

#define CLASS_DEF(type_name) struct type_name; typedef struct type_name type_name;\
	extern unsigned int type_name##_class_type; \
	struct type_name 

//#define __NEW(args...) ,##args);};obj;})
//#define NEW(type) ({type * obj = new_alloc(type##_class_type);if(obj){ obj->init(obj __NEW
#define NEW(type) ({type * obj = new_alloc(type##_class_type);obj;})
#define DELETE(obj) delete_free(obj)
#define __dynamic_cast(obj) (char *)(obj)))
#define dynamic_cast(type) ((type *) class_cast(type##_class_type, __dynamic_cast
#define __$(args...) ,##args)
#define $(obj,func) obj->func(obj __$


CLASS_DEF(object_t) {};

#ifdef __cplusplus
}
#endif

#endif /* __CHARSET_H__ */
