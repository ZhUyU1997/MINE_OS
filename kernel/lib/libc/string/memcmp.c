/*
 * libc/string/memcmp.c
 */

#include <types.h>
#include <string.h>
#include <core/module.h>

int memcmp(const void * s1, const void * s2, size_t n)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for (su1 = s1, su2 = s2; 0 < n; ++su1, ++su2, n--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

/*
 * Compares two buffers
 */
EXPORT_SYMBOL(memcmp);
