/*
 * libc/stdio/printf.c
 */

#include <stdarg.h>
#include <sizes.h>
#include <stdio.h>
#include <core/module.h>

static unsigned char buf[SZ_4K];

int printf(const char * fmt, ...)
{
	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = vsnprintf(buf, SZ_4K, fmt, ap);
	va_end(ap);

	int len = strlen((const char *)buf);
	for (int i = 0; i < len; i++)
		putchar(buf[i]);
	return rv;
}
EXPORT_SYMBOL(printf);
