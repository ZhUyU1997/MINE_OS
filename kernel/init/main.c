#include <stdio.h>
#include <interrupt.h>
#include <serial.h>
#include <assert.h>
#include <printk.h>
#include <softirq.h>
#include <schedule.h>
#include <irqflags.h>
#include <core/initcall.h>
#include <framebuffer/framebuffer.h>

int main() {

	init_memory();
	slab_init();

	do_init_class();

	printf("schedule_init\n");
	do_init_sched();

	/* Do initial event */
	do_init_event();

	/* Do all initial calls */
	do_initcalls();

	task_init();

	while (1);
	/* Do all exit calls */
	do_exitcalls();
	
}
