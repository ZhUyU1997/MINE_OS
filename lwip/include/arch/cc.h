#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__


/* 包含相关头文件 */
#include <string.h>
#include <assert.h>

#define BYTE_ORDER LITTLE_ENDIAN
#define	LWIP_PROVIDE_ERRNO
/* 提供标准错误代号 */
#define LWIP_PROVIDE_ERRNO


/* 定义与编译器无关的数据类型 */
typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   int     u32_t;
typedef signed     int     s32_t;


/* 定义指针类型 */
typedef u32_t mem_ptr_t;


/* 结构体 */
#define PACK_STRUCT_FIELD(x) x __attribute__((packed))
#define PACK_STRUCT_STRUCT  __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END


/* 平台调试输出和断言 */
#define LWIP_DEBUG
#define LWIP_PLATFORM_DIAG(x)	 printf x
#define LWIP_PLATFORM_ASSERT(x)  assert(x)

#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"

/* 临界区的保护( 使用uCOS-II的第三种临界处理方式 ) */
#include "os_cpu.h"
#define SYS_ARCH_DECL_PROTECT(x) u32_t cpu_sr
#define SYS_ARCH_PROTECT(x)      OS_ENTER_CRITICAL()
#define SYS_ARCH_UNPROTECT(x)    OS_EXIT_CRITICAL()

extern unsigned int sys_now(void);
#define LWIP_RAND() sys_now()

#endif /* __ARCH_CC_H__ */
