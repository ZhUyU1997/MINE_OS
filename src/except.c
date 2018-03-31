void HandleUndef() {
	unsigned int lr=0,cpsr=0,sp=0;
	asm volatile(
		"mrs	%1, spsr\n"
		"mrs	r0, cpsr\n"
		"mov	%0,lr\n"
		"msr	cpsr_c, #0xd3\n"
		"mov	%2,sp\n"
		"msr	cpsr_cxsf, r0\n"
		: "=r" (lr),"=r" (cpsr),"=r" (sp)
		:
		: "r0"
	);
	printf("\nHandleUndef!!!PC = %0X, CPSR = %0X, SP = %0X",lr,cpsr,sp);
	while (1);
}
void HandlePrefetchAbort() {
	unsigned int lr=0,cpsr=0,sp=0;
	asm volatile(
		"mrs	%1, spsr\n"
		"mrs	r0, cpsr\n"
		"mov	%0,lr\n"
		"msr	cpsr_c, #0xd3\n"
		"mov	%2,sp\n"
		"msr	cpsr_cxsf, r0\n"
		: "=r" (lr),"=r" (cpsr),"=r" (sp)
		:
		: "r0"
	);
	printf("\nHandleUndef!!!PC = %0X, CPSR = %0X, SP = %0X",lr,cpsr,sp);
	printf("\nHandlePrefetchAbort!!!");
	while (1);
}
void HandleDataAbort() {
	unsigned int lr=0,cpsr=0,sp=0;
	asm volatile(
		"mrs	%1, spsr\n"
		"mrs	r0, cpsr\n"
		"mov	%0,lr\n"
		"msr	cpsr_c, #0xd3\n"
		"mov	%2,sp\n"
		"msr	cpsr_cxsf, r0\n"
		: "=r" (lr),"=r" (cpsr),"=r" (sp)
		:
		: "r0"
	);
	printf("\nHandleUndef!!!PC = %0X, CPSR = %0X, SP = %0X",lr,cpsr,sp);
	printf("\nHandleDataAbort!!!");
	while (1);
}
