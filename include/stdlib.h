
#ifndef _STDLIB_H
#define _STDLIB_H
#ifndef NULL
#define NULL					((void*)0)
#endif
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
long simple_strtol(const char *cp,char **endp,unsigned int base);
int abs(int i);
long labs(long i);
#endif /* _STDIO_H */
