#ifndef __BUG_H__
#define __BUG_H__

#include <printk.h>
#include <compiler.h>

#define BUG() do { \
			color_printk(WHITE, RED, "BUG: %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
		} while (0)

#define BUG_ON(condition) do { \
			if (unlikely((condition)!=0)) \
				BUG(); \
		} while(0)

#define WARN_ON(condition) do{\
			if(unlikely((condition) != 0)) {\
				color_printk(YELLOW, BLACK, "WARN: %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
			}\
		}while(0)

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#endif

