/*
 * libc/stdio/putchar.c
 */

#include <stdio.h>
#include <core/module.h>

int putchar(int c)
{
	if (c == '\n') {
		int t = '\r';
		console_stdout_write(&t, 1);
	}
	console_stdout_write(&c, 1);
	return c;
}
EXPORT_SYMBOL(putchar);
