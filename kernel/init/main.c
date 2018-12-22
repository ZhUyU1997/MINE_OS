#include <stdio.h>
#include <interrupt.h>
#include <serial.h>
#include <assert.h>
#include <printk.h>
#include <softirq.h>
#include <schedule.h>
#include <irqflags.h>
#include <core/initcall.h>

#include <s3c24xx.h>
#include "framebuffer.h"

void show_bss_info(){
	extern char _bss, _ebss;
    unsigned int size = &_ebss - &_bss;
	printf("BSS大小为：%uKB, %uMB\n", size/1024, size/1024/1024);
}

int main() {
	irq_init();
	softirq_init();
	port_init();
	uart0_init();

	show_bss_info();

	printf("初始化MMU...\n");
	mmu_init();

	color_printk(RED, BLACK, "memory init \n");
	init_memory();

	color_printk(RED, BLACK, "slab init \n");
	slab_init();

	set_vector_map();
	color_printk(RED, BLACK, "schedule init \n");
	schedule_init();

	/* Do all initial calls */
	do_initcalls();
	printf("初始化TIMER...\n");
	s3c2440_timer_init();
	timer_init();

	printf("初始化TICK...\n");
	init_tick(50000);

	printf("初始化LCD...\n");
	lcd_init();
	lcd_enable();
	fb_get_lcd_params();
	ClearScreen(0);

	printf("初始化SD控制器...\n");
	SDI_init();
	task_init();
	local_irq_enable();

	cmd_loop();
	/* Do all exit calls */
	do_exitcalls();
	while (1);
}
