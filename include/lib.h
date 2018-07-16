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

#define NULL 0

#define container_of(ptr,type,member)							\
	({											\
		typeof(((type *)0)->member) * p = (ptr);					\
		(type *)((unsigned long)p - (unsigned long)&(((type *)0)->member));		\
	})


#define sti()
#define cli()
#define nop()
#define io_mfence()

#define hlt()
#define pause()

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


#define ALIGN(x,a)		(((x) + ((a) - 1)) & (~((a)-1)))
#define BIT_CLR(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] &=~(1<<((p) % (sizeof(char) * 8))))
#define BIT_SET(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] |= (1<<((p) % (sizeof(char) * 8))))
#define BIT_NOT(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] ^= (1<<((p) % (sizeof(char) * 8))))
#define ARRAY_SIZE(x)	((sizeof(x)) / (sizeof(x[0])))
#define LONG_BIT_NUM	(sizeof(long) * 8)

#define port_insw(port,buffer,nr)

#define port_outsw(port,buffer,nr)


#endif
