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

#ifndef __LIB_H__
#define __LIB_H__

#include <irqflags.h>
#include <global_config.h>

#define NULL 0


#define sti() local_irq_enable()
#define cli() local_irq_disable()
//#define nop()
//#define hlt()
//#define pause()

struct List {
	struct List * prev;
	struct List * next;
};

static inline void list_init(struct List * list) {
	list->prev = list;
	list->next = list;
}

static inline void list_add_to_behind(struct List * entry, struct List * new) {	////add to entry behind
	new->next = entry->next;
	new->prev = entry;
	new->next->prev = new;
	entry->next = new;
}

static inline void list_add_to_before(struct List * entry, struct List * new) {	////add to entry behind
	new->next = entry;
	entry->prev->next = new;
	new->prev = entry->prev;
	entry->prev = new;
}

static inline void list_del(struct List * entry) {
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
}

static inline long list_is_empty(struct List * entry) {
	if (entry == entry->next && entry->prev == entry)
		return 1;
	else
		return 0;
}

static inline struct List * list_prev(struct List * entry) {
	if (entry->prev != NULL)
		return entry->prev;
	else
		return NULL;
}

static inline struct List * list_next(struct List * entry) {
	if (entry->next != NULL)
		return entry->next;
	else
		return NULL;
}

#define offsetof(TYPE, MEMBER) ((unsigned int) &((TYPE *)0)->MEMBER)
#define container_of(ptr,type,member)	({	\
	const typeof(((type *)0)->member) * p = (ptr);					\
		(type *)((unsigned long)p - offsetof(type,member));})

#define list_entry(ptr,type,member)	\
	container_of(ptr, type, member)

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define ALIGN(x,a)		(((x) + ((a) - 1)) & (~((a)-1)))
#define BIT_CLR(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] &= ~(1<<((p) % (sizeof(char) * 8))))
#define BIT_SET(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] |=  (1<<((p) % (sizeof(char) * 8))))
#define BIT_NOT(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] ^=  (1<<((p) % (sizeof(char) * 8))))
#define ARRAY_SIZE(x)	((sizeof(x)) / (sizeof(x[0])))
#define BITS_PER_LONG	(sizeof(long) * 8)

static inline long verify_area(unsigned char* addr, unsigned long size) {
	//TODO:由于内核命令行需要测试系统调用，所以暂时统一返回1
#if 0
	if (((unsigned long)addr + size) <= TASK_SIZE)
		return 1;
	else
		return 0;
#else
	return 1;
#endif
}

static inline long copy_from_user(void * from, void * to, unsigned long size) {
	unsigned long d0, d1;
	if (!verify_area(from, size))
		return 0;
	memcpy(to, from, size);
	return size;
}

static inline long copy_to_user(void * from, void * to, unsigned long size) {
	unsigned long d0, d1;
	if (!verify_area(to, size))
		return 0;
	memcpy(to, from, size);
	return size;
}

static inline long strncpy_from_user(void * from, void * to, unsigned long size) {
	if (!verify_area(from, size))
		return 0;

	strncpy(to, from, size);
	return	size;
}

static inline long strnlen_user(void * src, unsigned long maxlen) {
	unsigned long size = strlen(src);
	if (!verify_area(src, size))
		return 0;

	return size <= maxlen ? size : maxlen;
}

#endif
