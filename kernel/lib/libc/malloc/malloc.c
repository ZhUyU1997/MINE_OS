/*
 * lib/libc/malloc/malloc.c
 */

#include <assert.h>
#include <spinlock.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <core/module.h>

static spinlock_t __heap_lock = SPIN_LOCK_INIT();

void * malloc(size_t size)
{
	void * m;

	spin_lock(&__heap_lock);
	m = kmalloc(size, 0);
	spin_unlock(&__heap_lock);
	return m;
}
EXPORT_SYMBOL(malloc);

void * memalign(size_t align, size_t size)
{
	void * m;

	spin_lock(&__heap_lock);
	m = kmalloc(size, 0);
	if(((size_t)m) & (align - 1)) {
		kfree(m);
		return NULL;
	}
	spin_unlock(&__heap_lock);
	return m;
}
EXPORT_SYMBOL(memalign);

/* TODO:
void * realloc(void * ptr, size_t size)
{
	void * m;

	spin_lock(&__heap_lock);
	m = tlsf_realloc(__heap_pool, ptr, size);
	spin_unlock(&__heap_lock);
	return m;
}
EXPORT_SYMBOL(realloc);
*/

void * calloc(size_t nmemb, size_t size)
{
	void * m;

	if((m = malloc(nmemb * size)))
		memset(m, 0, nmemb * size);
	return m;
}
EXPORT_SYMBOL(calloc);

void free(void * ptr)
{
	if(!ptr)
		return;
	spin_lock(&__heap_lock);
	kfree(ptr);
	spin_unlock(&__heap_lock);
}
EXPORT_SYMBOL(free);
