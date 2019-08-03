/*
 * libc/stdlib/labs.c
 */

#include <stdlib.h>
#include <core/module.h>

long labs(long n)
{
	return ((n < 0L) ? -n : n);
}
EXPORT_SYMBOL(labs);
