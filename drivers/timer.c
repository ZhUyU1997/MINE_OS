#include <s3c24xx.h>
#include "interrupt.h"


void timer_init() {
	TCFG0 |= (24);		//定时器 0，1 的预分频值
	TCFG0 |= (24 << 8);	//定时器 2，3 和 4 的预分频值
}
void tick_irq_hander();
void init_tick(int time, void (*handle)()) {
	//TCON:定时器控制寄存器
	TCON &= (~(7 << 20));		//清空20~21位
	TCON |= (1 << 22);		//定时器4间隙模式/自动重载
	TCON |= (1 << 21);		//定时器4手动更新TCNTB4

	//TCONB4:定时器4计数缓冲寄存器
	TCNTB4 = time;

	TCON |= (1 << 20);		//启动
	TCON &= ~(1 << 21);		//定时器4取消手动更新

	set_irq_handler(INT_TIMER4, handle);
	INTMSK_set(INT_TIMER4);
}

static volatile int tick = 0;

int get_sys_tick() {
	return tick;
}

void tick_irq_hander() {
	tick ++;
}

static void (*timer_handle)() = 0;

static void timer_handler() {
	if (timer_handle)
		timer_handle();
	INTMSK_clr(INT_TIMER1);
	TCON &= ~(1 << 8); //关闭
}

void set_timer(int time, void (*handle)()) {
	INTMSK_clr(INT_TIMER1);
	TCON &= ~(1 << 8); //关闭
	//TCON:定时器控制寄存器
	TCON &= (~(15 << 8));	//清空8~11位
	TCON |= (0 << 11);		//定时器1单稳态
	TCON |= (1 << 9);		//定时器1手动更新TCNTB1和TCMPB1

	//TCONB1:定时器1计数缓冲寄存器
	TCNTB1 = 1000 * time;

	TCON |= (1 << 8);		//启动
	TCON &= ~(1 << 9);		//定时器1取消手动更新

	if (handle)
		timer_handle = handle;
	set_irq_handler(INT_TIMER1, timer_handler);
	INTMSK_set(INT_TIMER1);
}

void close_timer() {
	INTMSK_clr(INT_TIMER1);
	TCON &= ~(1 << 8); //关闭
}

static volatile int delay_start = 0;
static void delay_irq_hander();

void udelay(int delay_time) {
	INTMSK_clr(INT_TIMER2);
	TCON &= ~(1 << 12); //关闭

	if (delay_time > 65535)
		delay_time = 65535;
	delay_start = 1;

	//TCON:定时器控制寄存器
	TCON &= (~(15 << 12));	//清空8~11位
	TCON |= (0 << 15);		//定时器2单稳态
	TCON |= (1 << 13);		//定时器2手动更新TCNTB2和TCMPB2

	//TCONB1:定时器2计数缓冲寄存器
	TCNTB1 = delay_time;

	TCON |= (1 << 12);		//启动
	TCON &= ~(1 << 13);		//定时器2取消手动更新

	set_irq_handler(INT_TIMER2, delay_irq_hander);
	//TODO:1多进程中断嵌套,2总开关不能随便用
	INTMSK_set(INT_TIMER2);
	while (delay_start != 0);
	INTMSK_clr(INT_TIMER2);
}

static void delay_irq_hander() {
	delay_start = 0;
	INTMSK_clr(INT_TIMER2);
	TCON &= (~(1 << 12)); //定时器关闭
}
