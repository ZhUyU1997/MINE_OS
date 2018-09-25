#include <irqflags.h>
void HandleUndef() {
	unsigned int pc = 0, cpsr = 0, sp = 0, lr = 0, fp = 0;
	asm volatile(
	    "mrs	%1, spsr\n"
	    "mrs	r0, cpsr\n"
	    "mov	%0, lr\n"
		"mov	%4, fp\n"
	    "msr	cpsr_c, #0xd3\n"
	    "mov	%2, sp\n"
	    "mov	%3, lr\n"
	    "msr	cpsr_cxsf, r0\n"
	    : "=r" (pc), "=r" (cpsr), "=r" (sp), "=r" (lr), "=r" (fp)
	    :
	    : "r0"
	);
	printf("\n%s!!!PC = %0X, CPSR = %0X, SP = %0X, LR = %0X, FP = %0X\n", __FUNCTION__, pc, cpsr, sp, lr, fp);
	backtrace(fp);
	while (1);
}
void HandlePrefetchAbort() {
	unsigned int pc = 0, cpsr = 0, sp = 0, lr = 0, fp = 0;
	asm volatile(
	    "mrs	%1, spsr\n"
	    "mrs	r0, cpsr\n"
	    "mov	%0, lr\n"
		"mov	%4, fp\n"
	    "msr	cpsr_c, #0xd3\n"
	    "mov	%2, sp\n"
	    "mov	%3, lr\n"
	    "msr	cpsr_cxsf, r0\n"
	    : "=r" (pc), "=r" (cpsr), "=r" (sp), "=r" (lr), "=r" (fp)
	    :
	    : "r0"
	);
	printf("\n%s!!!PC = %0X, CPSR = %0X, SP = %0X, LR = %0X, FP = %0X\n", __FUNCTION__, pc - 4, cpsr, sp, lr, fp);
	backtrace(fp);
	while (1);
}
void HandleDataAbort() {
	unsigned int pc = 0, cpsr = 0, sp = 0, lr = 0, fp = 0;
	asm volatile(
	    "mrs	%1, spsr\n"
	    "mrs	r0, cpsr\n"
	    "mov	%0, lr\n"
		"mov	%4, fp\n"
	    "msr	cpsr_c, #0xd3\n"
	    "mov	%2, sp\n"
	    "mov	%3, lr\n"
	    "msr	cpsr_cxsf, r0\n"
	    : "=r" (pc), "=r" (cpsr), "=r" (sp), "=r" (lr), "=r" (fp)
	    :
	    : "r0"
	);
	printf("\n%s!!!PC = %0X, CPSR = %0X, SP = %0X, LR = %0X, FP = %0X\n", __FUNCTION__, pc - 8, cpsr, sp, lr, fp);
	backtrace(fp);
	while (1);
}
