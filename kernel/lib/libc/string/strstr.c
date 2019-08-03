/*
 * libc/string/strstr.c
 */

#include <types.h>
#include <stddef.h>
#include <string.h>
#include <core/module.h>

/*
 * Finds the first occurrence of a substring in a string
 */
char * strstr(const char * s1, const char * s2)
{
	size_t l1, l2;

	l2 = strlen(s2);
	if (!l2)
		return (char *)s1;

	l1 = strlen(s1);
	while (l1 >= l2)
	{
		l1--;
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}

	return NULL;
}
EXPORT_SYMBOL(strstr);
