#include <global_config.h>
#include <interrupt.h>
#include <assert.h>
#include "s3c24xx.h"
#include "ucos_ii.h"
void (*isr_handle_array[50])(void);
void enable_irq(void) {
	__asm__ volatile(
		"mrs r0,cpsr\n"
		"bic r0,r0,#0xc0\n"
		"msr cpsr_c,r0\n"
		:::"r0"
	);
}
void disable_irq(void) {
	__asm__ volatile(
		"mrs r0,cpsr\n"
		"orr r0,r0,#0xc0\n"
		"msr cpsr_c,r0\n"
		:::"r0"
	);
}

static void set_int(enum INT_NUM offset) {
	assert((offset >= EINT0) && (offset <= INT_ADC));
	INTMSK &= ~(1 << offset);
}
static void clr_int(enum INT_NUM offset) {
	assert((offset >= EINT0) && (offset <= INT_ADC));
	INTMSK |= (1 << offset);
}

void set_subint(enum INT_SUB_NUM offset) {
	assert((offset >= INT_RXD0) && (offset <= INT_AC97));
	INTSUBMSK &= ~(1 << offset);
}
void clr_subint(enum INT_SUB_NUM offset) {
	assert((offset >= INT_RXD0) && (offset <= INT_AC97));
	INTSUBMSK |= (1 << offset);
}
void dummy_isr(void) {
	printf("IRQ HANDLE,ERROR!\n");
	assert(0);
}

void irq_init(void) {
	for (int i = 0; i < sizeof(isr_handle_array) / sizeof(isr_handle_array[0]); i++) {
		isr_handle_array[i] = dummy_isr;
	}

	INTMOD = 0x0;	      //所有中断都设为IRQ模式
	INTMSK = BIT_ALLMSK;  //先屏蔽所有中断
	SRCPND = 0xffffffff;
	INTPND = 0xffffffff;
	SUBSRCPND = 0xffffffff;
	INTSUBMSK = 0xffff;
	EINTMASK = 0xffff << 4;
	EINTPEND = 0xffff << 4;
	memcpy(0x33ff0000, 0x30100000, 4096);
}
static void clear_pending(enum INT_NUM offset);

static U32 subsrcpnd;
void save_subsrcpnd(U32 reg) {
	subsrcpnd = reg;
}

U32 get_subsrcpnd() {
	return subsrcpnd;
}

void do_irq(void) {
	enum INT_NUM offset = (enum INT_NUM)INTOFFSET;
	save_subsrcpnd(SUBSRCPND);
#ifdef CONFIG_UCOS2
	OSIntEnter();
#endif
	clear_pending(offset);
	isr_handle_array[offset]();
#ifdef CONFIG_UCOS2
	OSIntExit();
#endif
}

void request_irq(enum INT_NUM offset, int (*handler)(void)) {
	assert((offset >= EINT0) && (offset <= INT_ADC) && handler);
	if (handler != 0)
		isr_handle_array[offset] = handler;
	else
		isr_handle_array[offset] = dummy_isr;
	clear_pending(offset);
	set_int(offset);
}

void free_irq(enum INT_NUM offset) {
	assert((offset >= EINT0) && (offset <= INT_ADC));
	clr_int(offset);
	isr_handle_array[offset] = dummy_isr;
	clear_pending(offset);
}

//清中断
static void clear_pending(enum INT_NUM offset) {
	assert((offset >= EINT0) && (offset <= INT_ADC));
	//TODO:不通用
	if (offset == EINT4_7) {
		//EINT4-7合用IRQ4，注意EINTPEND[3:0]保留未用，向这些位写入1可能导致未知结果
		EINTPEND = 1 << EINT7;
	}
	if (offset == EINT8_23) {
		//EINT8_23合用IRQ5
		EINTPEND = 1 << 11;
	}
	if (offset == INT_ADC) {
		if (SUBSRCPND & (1 << INT_ADC_S))
			SUBSRCPND = 1 << INT_ADC_S;
		if (SUBSRCPND & (1 << INT_TC))
			SUBSRCPND = 1 << INT_TC;
	}
	if (offset == INT_UART0) {
		if (SUBSRCPND & (1 << INT_RXD0))
			SUBSRCPND = 1 << INT_RXD0;
		if (SUBSRCPND & (1 << INT_TXD0))
			SUBSRCPND = 1 << INT_TXD0;
	}

	SRCPND = 1 << offset;
	INTPND = 1 << offset;
}
