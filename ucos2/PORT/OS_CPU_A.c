#include <ucos_ii.h>
void OSStartHighRdy(){
	OSTaskSwHook();
	OSRunning = OS_TRUE;
	OS_STK *SP = OSTCBHighRdy->OSTCBStkPtr;
	__asm__ volatile (
	    "MOV	SP,%0\n"
	    "LDMFD	SP!, {R0}\n"
		"MSR	SPSR_cxsf, R0\n"
		"LDMFD	SP!, {R0-R12, LR, PC}^\n"
		:
		:"r"(SP)
		:
	);
}

void OSTickISR(){
	OSTimeTick();
}

OS_CPU_SR OSCPUSaveSR(void)
{
	OS_CPU_SR flags, temp;
	
	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_save\n"
		"	orr	%1, %0, #0x80\n"
		"	msr	cpsr_c, %1"//其它域不变
		: "=r" (flags), "=r" (temp)
		:
		: "memory", "cc");
	return flags;
}


void  OSCPURestoreSR(OS_CPU_SR cpu_sr){
	asm volatile(
		"	msr	cpsr_c, %0	@ local_irq_restore"//其它域不变
		"	msr	cpsr_c, %0	@ local_irq_restore"//其它域不变
		:
		: "r" (cpu_sr)
		: "memory", "cc");
}
