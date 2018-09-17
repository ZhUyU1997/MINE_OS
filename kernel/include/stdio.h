#ifndef __STDIO_H__
#define __STDIO_H__
#define stdin	0
#define stdout	1
#define stderr	2
#include "sys/types.h"

#define	SEEK_SET	0	/* Seek relative to start-of-file */
#define	SEEK_CUR	1	/* Seek relative to current position */
#define	SEEK_END	2	/* Seek relative to end-of-file */

#define SEEK_MAX	3

#include <stdarg.h>

extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
extern int snprintf(char * buf, size_t size, const char *fmt, ...);
extern int vsprintf(char *buf, const char *fmt, va_list args);
extern int sprintf(char * buf, const char *fmt, ...);
extern int vsscanf(const char * buf, const char * fmt, va_list args);
extern int sscanf(const char * buf, const char * fmt, ...);

extern void putc(unsigned char c);
extern unsigned char getc(void);

int printf(const char *fmt, ...);
int scanf(const char * fmt, ...);

#endif /* _STDIO_H */
