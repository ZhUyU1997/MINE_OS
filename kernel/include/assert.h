#ifndef __ASSERT_H__
#define __ASSERT_H__
#ifdef NDEBUG
#define assert(test) ((void)0)
#else
void __Assert(char *);
#define _STR(x) _VAR(x)
#define _VAR(x) #x
#define assert(test) ((test)? ((void)0):__Assert(_STR(test)", file: "__FILE__",line: "_STR(__LINE__)))
#endif
#endif