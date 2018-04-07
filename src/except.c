void HandleUndef() {
	unsigned int pc = 0, cpsr = 0, sp = 0, lr = 0;
	asm volatile(
	    "mrs	%1, spsr\n"
	    "mrs	r0, cpsr\n"
	    "mov	%0,lr\n"
	    "msr	cpsr_c, #0xd3\n"
	    "mov	%2,sp\n"
	    "mov	%3,lr\n"
	    "msr	cpsr_cxsf, r0\n"
	    : "=r" (pc), "=r" (cpsr), "=r" (sp), "=r" (lr)
	    :
	    : "r0"
	);
	printf("\nHandleUndef!!!PC = %0X, CPSR = %0X, SP = %0X, LR = %0X", pc, cpsr, sp, lr);
	while (1);
}
void HandlePrefetchAbort() {
	unsigned int pc = 0, cpsr = 0, sp = 0, lr = 0;
	asm volatile(
	    "mrs	%1, spsr\n"
	    "mrs	r0, cpsr\n"
	    "mov	%0,lr\n"
	    "msr	cpsr_c, #0xd3\n"
	    "mov	%2,sp\n"
	    "mov	%3,lr\n"
	    "msr	cpsr_cxsf, r0\n"
	    : "=r" (pc), "=r" (cpsr), "=r" (sp), "=r" (lr)
	    :
	    : "r0"
	);
	printf("\nHandlePrefetchAbort!!!PC = %0X, CPSR = %0X, SP = %0X, LR = %0X", pc - 4, cpsr, sp, lr);
	while (1);
}
void HandleDataAbort() {
	unsigned int pc = 0, cpsr = 0, sp = 0, lr = 0;
	asm volatile(
	    "mrs	%1, spsr\n"
	    "mrs	r0, cpsr\n"
	    "mov	%0,lr\n"
	    "msr	cpsr_c, #0xd3\n"
	    "mov	%2,sp\n"
	    "mov	%3,lr\n"
	    "msr	cpsr_cxsf, r0\n"
	    : "=r" (pc), "=r" (cpsr), "=r" (sp), "=r" (lr)
	    :
	    : "r0"
	);
	printf("\nHandleDataAbort!!!PC = %0X, CPSR = %0X, SP = %0X, LR = %0X\n", pc - 8, cpsr, sp, lr);
	while (1);
}
