#include <interrupt.h>
#include <assert.h>
#include <irqs.h>
#include <bug.h>
#include <ptrace.h>
#include <preempt.h>
#include <softirq.h>
#include <schedule.h>

void handle_IRQ(unsigned int irq, struct pt_regs *regs) {
	generic_handle_irq(irq);
	do_softirq();
	assert(preempt_count()>=0);
	if(!preempt_count())
		schedule();
}

void do_irq(unsigned int irq, struct pt_regs *regs) {
	handle_IRQ(irq, regs);
}
