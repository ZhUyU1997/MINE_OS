#include <interrupt.h>
#include <assert.h>
#include <irqs.h>
#include <kernel.h>
#include <schedule.h>
#include <preempt.h>

void arch_irq_handler_default(struct pt_regs *regs) {
	arm32_do_irq();
	do_softirq();
	assert(preempt_count()>=0);
	if(!preempt_count())
		schedule();
}
