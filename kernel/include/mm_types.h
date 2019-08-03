#ifndef __MM_TYPES_H__
#define __MM_TYPES_H__

#include <types.h>
#include <pgtable-2level-types.h>

struct vm_area_struct {

};

struct mm_struct {
	pgd_t *pgd;	//page table point

	unsigned long start_code, end_code;
	unsigned long start_data, end_data;
	unsigned long start_rodata, end_rodata;
	unsigned long start_bss, end_bss;
	unsigned long start_brk, end_brk;
	unsigned long start_stack;
};

#endif
