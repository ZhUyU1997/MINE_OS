#include <stdio.h>
#include <printk.h>
#include <task.h>
#include <schedule.h>
#include <irqflags.h>
#include <core/initcall.h>
#include <framebuffer/framebuffer.h>

int main() {

	do_init_memory();

	do_init_slab();

	do_init_class();

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
