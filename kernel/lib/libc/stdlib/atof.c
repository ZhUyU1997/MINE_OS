/*
 * libc/stdlib/atof.c
 */

#include <stdlib.h>
#include <core/module.h>

double atof(const char * nptr)
{
	return (double)strtod(nptr, 0);
}
EXPORT_SYMBOL(atof);
