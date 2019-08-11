#ifndef __STDIO_H__
#define __STDIO_H__

#define stdin	0
#define stdout	1
#define stderr	2

#include <types.h>

#ifndef EOF
#define EOF			(-1)
#endif

enum {
	SEEK_SET		= 0,		/* set file offset to offset */
	SEEK_CUR		= 1,		/* set file offset to current plus offset */
	SEEK_END		= 2,		/* set file offset to EOF plus offset */
};


#include <stdarg.h>
#include <stddef.h>

extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
extern int snprintf(char * buf, size_t size, const char *fmt, ...);
extern int vsprintf(char *buf, const char *fmt, va_list args);
extern int sprintf(char * buf, const char *fmt, ...);
extern int vsscanf(const char * buf, const char * fmt, va_list args);
extern int vasprintf(char ** s, const char * fmt, va_list ap);
extern int sscanf(const char * buf, const char * fmt, ...);

int putchar(int c);
int getchar(void);

int printf(const char *fmt, ...);
int scanf(const char * fmt, ...);

#endif /* _STDIO_H */
