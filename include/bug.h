#ifndef __BUG_H__
#define __BUG_H__

#define BUG() do { \
			printf("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
			panic("BUG!"); \
		} while (0)

#define BUG_ON(condition) do { \
		if (unlikely((condition)!=0)) \
			BUG(); \
		} while(0)

#define WARN_ON(condition) do{\
		if(unlikely((condition) != 0)) {\
			printf(鈥淏adness in %s at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
			dump_stack();\
		}\
}while(0)

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#endif

