#include "s3c24xx.h"
#include "ucos_ii.h"
//extern void I2CIntHandle(void);
void ClearPending(unsigned bit);
void (*isr_handle_array[50])(void);

void enable_irq(void) {
	__asm__ volatile (
	    "mrs r4,cpsr\n"
	    "bic r4,r4,#0xc0\n"
	    "msr cpsr_c,r4\n"
	    :::"r4"
	);
}
void disable_irq(void) {
	__asm__ volatile (
	    "mrs r4,cpsr\n"
	    "orr r4,r4,#0xc0\n"
	    "msr cpsr_c,r4\n"
	    :::"r4"
	);
}
//TODO:INTMS改为INTMSK
void INTMS_set(unsigned int offset) {
	INTMSK &= ~(1 << offset);
}
void INTMS_clr(unsigned int offset) {
	INTMSK |= (1 << offset);
}
void Dummy_isr(void) {
	printf("IRQ HANDLE!\n\r");
	while (1);
}

void init_irq(void) {
	int i = 0;
	for (i = 0; i < sizeof(isr_handle_array) / sizeof(isr_handle_array[0]); i++) {
		isr_handle_array[i] = Dummy_isr;
	}

	INTMOD = 0x0;	      // 所有中断都设为IRQ模式
	INTMSK = BIT_ALLMSK;  // 先屏蔽所有中断

//	isr_handle_array[ISR_IIC_OFT]  = I2CIntHandle;
}

void IRQ_Handle(void) {
	unsigned long oft = INTOFFSET;
	//清中断
	ClearPending(INTOFFSET);
	//printf("INTOFFSET:%d\n",oft);

	//TODO:中断嵌套时，中断开关设置
	/* 调用中断服务程序 */
	OSIntEnter();
	isr_handle_array[oft]();
	OSIntExit();
}
void set_irq_handler(int offset, int (*handler)(void)) {
	//TODO:完善
	if (handler != 0)
		isr_handle_array[offset] = handler;
	else
		isr_handle_array[offset] = Dummy_isr;
}
//清中断
void ClearPending(unsigned oft) {
	if (oft == 4)
		EINTPEND = 1 << 7;  //EINT4-7合用IRQ4，注意EINTPEND[3:0]保留未用，向这些位写入1可能导致未知结果
	if (oft == 5)
		EINTPEND = 1 << 11; // EINT8_23合用IRQ5
	SRCPND = 1 << oft;
	INTPND = 1 << oft;
}
