#ifndef __DEF_H__
#define __DEF_H__

#ifndef U32
typedef unsigned int u32_t;
#endif

#ifndef U16
typedef unsigned short u16_t;
#endif

#ifndef S32
typedef int S32;
#endif

#ifndef S16
typedef short int S16;
#endif

#ifndef U8
typedef unsigned char u8_t;
#endif

#ifndef S8
typedef char S8;
#endif

typedef char *LPSTR;

typedef unsigned char BOOL;

typedef unsigned char UCHAR;
typedef unsigned char *PUCHAR;

typedef unsigned char BYTE;
typedef unsigned char *LPBYTE;

typedef unsigned short WORD;
typedef unsigned short *PDWORD;

typedef unsigned short USHORT;
typedef unsigned short *PUSHORT;

typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef unsigned long *PULONG;

typedef unsigned long LDWORD;
typedef unsigned long *LPDWORD;

typedef void *PVOID;

#ifndef NULL
#define NULL 0
#endif

#define TRUE 	1
#define FALSE 	0

#endif /*__DEF_H__*/


