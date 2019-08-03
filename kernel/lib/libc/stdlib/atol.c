/*
 * libc/stdlib/atol.c
 */

#include <stddef.h>
#include <stdlib.h>
#include <core/module.h>

long atol(const char * nptr)
{
	return (long)strtol(nptr, NULL, 10);
}
EXPORT_SYMBOL(atol);
