#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <sys/types.h>

void PutPixel(UINT32 x, UINT32 y, UINT16 color);
UINT16 GetPixel(UINT32 x, UINT32 y);

#endif /*__FRAMEBUFFER_H__*/
