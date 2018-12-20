#include <stdio.h>
#include <s3c24xx.h>
#include <interrupt.h>
#include <serial.h>
#include <assert.h>
#include <printk.h>
#include <fcntl.h>
#include <softirq.h>
#include <schedule.h>
#include <irqflags.h>

#include "framebuffer.h"

void show_bss_info(){
	extern char __bss_start, __bss_end;
    unsigned int size = &__bss_end - &__bss_start;
	printf("BSS大小为：%uKB, %uMB\n", size/1024, size/1024/1024);
}
static void initer(void (*init)(), char *msg){
	assert(init);
	if(init){
		if(msg)
			printf(msg);
		init();
	}
}

int main() {
	irq_init();

	color_printk(RED, BLACK, "Soft IRQ init \n");
	softirq_init();

	port_init();
	uart0_init();
	//uart0_interrupt_init();
	printf("\n\n************************************************\n");
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

	printf("使能IRQ...\n");
	local_irq_enable();

	cmd_loop();
	while (1);
}
