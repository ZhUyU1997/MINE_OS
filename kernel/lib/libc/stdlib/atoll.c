/*
 * libc/stdlib/atoll.c
 */

#include <stddef.h>
#include <stdlib.h>
#include <core/module.h>

long long atoll(const char * nptr)
{
	return (long long)strtoll(nptr, NULL, 10);
}
EXPORT_SYMBOL(atoll);
