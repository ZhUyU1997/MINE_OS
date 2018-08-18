#ifndef __MMU_H__
#define  __MMU_H__

#define MMU_FULL_ACCESS     (3)			/* ����Ȩ�� */
#define MMU_DOMAIN(x)       (x)			/* �����ĸ��� */
#define MMU_SPECIAL         (1)			/* ������1 */
#define MMU_CACHE_ENABLE    (1)			/* cache enable */
#define MMU_BUFFER_ENABLE   (1)			/* buffer enable */
#define MMU_CACHE_DISABLE   (0)			/* cache disable */
#define MMU_BUFFER_DISABLE  (0)			/* buffer disable */

#define MMU_COARSE_PAGE     (1)         /* ��ʾ���Ǵ�ҳ������ */
#define MMU_SECTION         (2)         /* ��ʾ���Ƕ������� */
#define MMU_FINE_PAGE		(3)         /* ��ʾ����ϸҳ������ */

#define MMU_LARGE_PAGE 	    (1)         /* ��ʾ���Ǵ�ҳ������ */
#define MMU_SMALL_PAGE		(2)         /* ��ʾ����Сҳ������ */
#define MMU_TINY_PAGE		(3)         /* ��ʾ���Ǽ�Сҳ������ */

#define MMU_COARSE_PAGE_SHIFT		(10)
#define MMU_SECTION_SHIFT			(20)
#define MMU_FINE_PAGE_SHIFT			(12)

#define MMU_LARGE_PAGE_SHIFT 	    (16)
#define MMU_SMALL_PAGE_SHIFT		(12)
#define MMU_TINY_PAGE_SHIFT			(10)

#define MMU_SECTION_SIZE	(1<<MMU_SECTION_SHIFT)
#define MMU_LARGE_PAGE_SIZE	(1<<MMU_LARGE_PAGE_SHIFT)
#define MMU_SMALL_PAGE_SIZE	(1<<MMU_SMALL_PAGE_SHIFT)
#define MMU_TINY_PAGE_SIZE 	(1<<MMU_TINY_PAGE_SHIFT)


typedef struct COARSE_PAGE {
	unsigned int type: 2;
	unsigned int blank1: 2;
	unsigned int special: 1;
	unsigned int domain: 4;
	unsigned int blank0: 1;
	unsigned int base_address: 22;
} COARSE_PAGE;

typedef struct SECTION {
	unsigned int type: 2;
	unsigned int B: 1;
	unsigned int C: 1;
	unsigned int special: 1;
	unsigned int domain: 4;
	unsigned int blank1: 1;
	unsigned int AP: 2;
	unsigned int blank0: 8;
	unsigned int base_address: 12;
} SECTION;

typedef struct FINE_PAGE {
	unsigned int type: 2;
	unsigned int blank1: 2;
	unsigned int special: 1;
	unsigned int domain: 4;
	unsigned int blank0: 3;
	unsigned int base_address: 20;
} FINE_PAGE;

typedef struct LARGE_PAGE {
	unsigned int type: 2;
	unsigned int B: 1;
	unsigned int C: 1;
	unsigned int AP0: 2;
	unsigned int AP1: 2;
	unsigned int AP2: 2;
	unsigned int AP3: 2;
	unsigned int blank0: 4;
	unsigned int base_address: 16;
} LARGE_PAGE;

typedef struct SMALL_PAGE {
	unsigned int type: 2;
	unsigned int B: 1;
	unsigned int C: 1;
	unsigned int AP0: 2;
	unsigned int AP1: 2;
	unsigned int AP2: 2;
	unsigned int AP3: 2;
	unsigned int base_address: 20;
} SMALL_PAGE;

typedef struct TINY_PAGE {
	unsigned int type: 2;
	unsigned int B: 1;
	unsigned int C: 1;
	unsigned int AP: 2;
	unsigned int blank0: 4;
	unsigned int base_address: 22;
} TINY_PAGE;

#endif
