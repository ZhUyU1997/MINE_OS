#include <global_config.h>
#include <interrupt.h>
#include <assert.h>
#include <irqs.h>
#include <bug.h>
#include <s3c24xx.h>
#include <ptrace.h>
#include <preempt.h>
#include <softirq.h>
#include <schedule.h>

static void dummy_isr(unsigned long nr, unsigned long parameter) {
	printf("IRQ HANDLE,ERROR!\n");
	assert(0);
}

struct irq_desc interrupt_desc[NR_IRQS] = {
	[0 ... NR_IRQS - 1] = {
		.controller = NULL,
		.handler = dummy_isr,
	}
};

static void unmask_irq(int irq) {
	assert(interrupt_desc[irq].controller&&interrupt_desc[irq].controller->unmask);
	interrupt_desc[irq].controller->unmask(irq);
}
static void mask_irq(int irq) {
	assert(interrupt_desc[irq].controller&&interrupt_desc[irq].controller->unmask);
	interrupt_desc[irq].controller->mask(irq);
}

//清中断
static void ack_irq(int irq) {
	interrupt_desc[irq].controller->ack(irq);
}

void handle_IRQ(unsigned int irq, struct pt_regs *regs) {
	generic_handle_irq(irq);
	do_softirq();
	assert(preempt_count()>=0);
	if(!preempt_count())
		schedule();
}

void do_irq(struct pt_regs *regs) {
	int offset = INTOFFSET;
	unsigned int irq = offset + S3C2440_CPUIRQ_OFFSET;
	assert((irq >= IRQ_EINT0) && (irq <= IRQ_ADCPARENT));
	handle_IRQ(irq, regs);
}

void request_irq(int irq, irq_handler_t handler) {
	assert((irq >= IRQ_EINT0) && (irq < NR_IRQS) && handler);
	if (handler && (interrupt_desc[irq].handler == dummy_isr))
		interrupt_desc[irq].handler = handler;
	else
		assert(0);
	unmask_irq(irq);
}

void free_irq(int irq) {
	assert((irq >= IRQ_EINT0) && (irq < NR_IRQS));
	WARN_ON(interrupt_desc[irq].handler == dummy_isr);
	mask_irq(irq);
	interrupt_desc[irq].handler = dummy_isr;
}


