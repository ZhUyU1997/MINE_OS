/*
 * libc/stdio/getchar.c
 */

#include <stdio.h>
#include <core/module.h>

int getchar(void)
{
	unsigned char res;
	while(console_stdin_read(&res, 1) <= 0);
	return res;
}
EXPORT_SYMBOL(getchar);
