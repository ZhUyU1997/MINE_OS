#include <s3c24xx.h>
#include "interrupt.h"

void delay_irq_hander();

void init_tick(int time, void (*handle)()) {
	//定时器配制寄存器 0 （TCFG0）
	TCFG0 |= (24 << 8); //定时器 2，3 和 4 的预分频值
	//定时器控制寄存器 1 （TCON）
	TCON &= (~(7 << 20)); //清空20~21位
	TCON |= (1 << 22); //定时器 4单稳态
	TCON |= (1 << 21); //定时器 4手动更新 TCNTB4
	//TCONB4:定时器 4  计数缓冲寄存器
	TCNTB4 = time;

	TCON |= (1 << 20); //启动
	TCON &= ~(1 << 21); //定时器 4 取消手动更新

	set_irq_handler(INT_TIMER4, handle);
	INTMSK_set(INT_TIMER4);
}
static volatile int tick = 0;
int get_sys_tick() {
	return tick;
}
void delay_irq_hander() {
	tick ++;
}
static void (*timer_handle)() = 0;
void timer_handler() {
	if (timer_handle)
		timer_handle();
	INTMSK_clr(INT_TIMER1);
	TCON &= ~(1 << 8); //关闭
}
void set_timer(int time, void (*handle)()) {
	INTMSK_clr(INT_TIMER1);
	TCON &= ~(1 << 8); //关闭
	//定时器配制寄存器 0 （TCFG0）
	TCFG0 |= (24); //定时器 0，1 的预分频值
	//定时器控制寄存器 1 （TCON）
	TCON &= (~(15 << 8)); //清空8~11位
	TCON |= (0 << 11); //定时器 1间隙模式（自动重载）
	TCON |= (1 << 9); //定时器 1手动更新 TCNTB1
	//TCONB1:定时器 1  计数缓冲寄存器
	TCNTB1 = 1000 * time;

	TCON |= (1 << 8); //启动
	TCON &= ~(1 << 9); //定时器 1 取消手动更新

	if (handle)
		timer_handle = handle;
	set_irq_handler(INT_TIMER1, timer_handler);
	INTMSK_set(INT_TIMER1);
}
void close_timer() {
	INTMSK_clr(INT_TIMER1);
	TCON &= ~(1 << 8); //关闭
}
