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

#include <types.h>
#include <string.h>
#include <list.h>
#include <irqflags.h>
#include <kernel.h>

#define sti() local_irq_enable()
#define cli() local_irq_disable()
//#define nop()
//#define hlt()
//#define pause()

#define BIT_CLR(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] &= ~(1<<((p) % (sizeof(char) * 8))))
#define BIT_SET(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] |=  (1<<((p) % (sizeof(char) * 8))))
#define BIT_NOT(x,p)	(((char *)(x))[(p) / (sizeof(char) * 8)] ^=  (1<<((p) % (sizeof(char) * 8))))
#define BITS_PER_LONG	(sizeof(long) * 8)

static inline long verify_area(unsigned char* addr, unsigned long size) {
	//TODO:由于内核命令行需要测试系统调用，所以暂时统一返回1
#if 0
	if (((unsigned long)addr + size) <= CONFIG_TASK_SIZE)
		return 1;
	else
		return 0;
#else
	return 1;
#endif
}

static inline long copy_from_user(void * to, void * from, unsigned long size) {
	if (!verify_area(from, size))
		return 0;
	memcpy(to, from, size);
	return size;
}

static inline long copy_to_user(void * to, void * from, unsigned long size) {
	if (!verify_area(to, size))
		return 0;
	memcpy(to, from, size);
	return size;
}

static inline long strncpy_from_user(void * to, void * from, unsigned long size) {
	if (!verify_area(from, size))
		return 0;

	strncpy(to, from, size);
	return size;
}

static inline long strnlen_user(void * src, unsigned long maxlen) {
	unsigned long size = strlen(src);
	if (!verify_area(src, size))
		return 0;

	return size <= maxlen ? size : maxlen;
}

#endif
