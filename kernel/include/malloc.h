#ifndef __MALLOC_H__
#define __MALLOC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

void * malloc(size_t size);
void * memalign(size_t align, size_t size);
void * realloc(void * ptr, size_t size);
void * calloc(size_t nmemb, size_t size);
void free(void * ptr);

#ifdef __cplusplus
}
#endif

#endif /* __MATH_H__ */
