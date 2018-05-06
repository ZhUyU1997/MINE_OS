#ifndef __TIMER_H__
#define __TIMER_H__

void timer_init();
void init_tick(unsigned int time, void (*handle)());
void tick_irq_hander();
int get_sys_tick();
void set_timer(unsigned int time, void (*handle)());
void delay_u(unsigned int delay_time);

static void udelay(unsigned int time){
	for(volatile int i=0;i<36;i++)
		for(volatile unsigned int i=0;i<time;i++);
}
#endif
