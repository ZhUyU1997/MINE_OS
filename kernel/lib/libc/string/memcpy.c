/*
 * libc/string/memcpy.c
 */

#include <types.h>
#include <string.h>
#include <core/module.h>

void * memcpy(void * dest, const void * src, size_t len)
{
	char * tmp = dest;
	const char * s = src;

	while (len--)
		*tmp++ = *s++;
	return dest;
}

/*
 * Copies one buffer to another
 */
EXPORT_SYMBOL(memcpy);
