#include <global_config.h>
#include "backtrace.h"

extern unsigned long kallsyms_addresses[] __attribute__((weak));;
extern long kallsyms_syms_num __attribute__((weak));;
extern long kallsyms_index[] __attribute__((weak));;
extern char* kallsyms_names __attribute__((weak));;

int lookup_kallsyms(unsigned long address) {
	int index = 0;
	char * string = (char *) &kallsyms_names;

	for (index = 0; index < kallsyms_syms_num; index++)
		if (address > kallsyms_addresses[index] && address <= kallsyms_addresses[index + 1])
			break;
	if (index < kallsyms_syms_num) {
		printf("\033[31;40mbacktrace address:%#08lx (+) %04d\tbacktrace function:%s(%#08lx)\033[0m\n", address, address - kallsyms_addresses[index], &string[kallsyms_index[index]], kallsyms_addresses[index]);
		return 0;
	} else {
		printf("\033[31;40mbacktrace address:%#08lx,Œ¥’“µΩ∑˚∫≈\033[0m\n", address);
		return 1;
	}
}

void backtrace(unsigned long regs) {
	unsigned long *rbp = (unsigned long *)regs;
	unsigned long ret_address;

	for (int i = 0; i < 10; i++) {
		if (rbp >= SWI_STACK_BASE_ADDR) {
			printf("\033[31;40mµΩ¥Ô’ª∂•\033[0m\n");
			break;
		}

		ret_address = *(rbp);
		if (lookup_kallsyms(ret_address))
			break;
		rbp = (unsigned long *) * (rbp - 1);
	}
}



