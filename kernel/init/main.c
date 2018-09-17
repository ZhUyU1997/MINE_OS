#include <stdio.h>
#include <s3c24xx.h>
#include <interrupt.h>
#include <serial.h>
#include <assert.h>
#include <printk.h>
#include <fcntl.h>

//lcd driver
#include "lcddrv.h"
#include "framebuffer.h"

//fs
#include "ff.h"
FATFS fatworkarea;         // Work area (file system object) for logical drives 

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

static void vt100_response(char *str){
	printf(str);
}

int main() {
	irq_init();

	color_printk(RED, BLACK, "Soft IRQ init \n");
	softirq_init();

	Port_Init();
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
	Lcd_Port_Init();						// 设置LCD引脚
	Tft_Lcd_Init(MODE_TFT_16BIT_480272);	// 初始化LCD控制
	Lcd_PowerEnable(0, 1);					// 设置LCD_PWREN有效，它用于打开LCD的电源
	Lcd_EnvidOnOff(1);						// 使能LCD控制器输出信号
	ClearScr(get_text_bgcolor());			// 清屏

	printf("初始化SD控制器...\n");
	SDI_init();

	printf("初始化fatfs...\n");
	f_mount(0,&fatworkarea);

	task_init();

	printf("使能IRQ...\n");
	enable_irq();

	cmd_loop();
	while (1);
}
