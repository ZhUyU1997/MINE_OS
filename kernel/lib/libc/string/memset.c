/*
 * libc/string/memset.c
 */

#include <types.h>
#include <string.h>
#include <core/module.h>

void * memset(void * s, int c, size_t n)
{
	char * xs = s;

	while (n--)
		*xs++ = c;

	return s;
}

/*
 * Fills a buffer with a repeated byte/wchar_t
 */
EXPORT_SYMBOL(memset);
