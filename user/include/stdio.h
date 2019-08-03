#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdarg.h>

enum {
	SEEK_SET		= 0,		/* set file offset to offset */
	SEEK_CUR		= 1,		/* set file offset to current plus offset */
	SEEK_END		= 2,		/* set file offset to EOF plus offset */
};

int printf(const char *fmt, ...);
int sprintf(char * buf,const char * fmt,...);
int vsprintf(char * buf,const char *fmt, va_list args);


#endif
