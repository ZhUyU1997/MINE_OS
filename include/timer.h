#ifndef __TIMER_H__
#define __TIMER_H__

void timer_init();
void init_tick(unsigned int time, void (*handle)());
int get_sys_tick();
unsigned long long get_system_time_us(void);
unsigned long long delta_time_us(unsigned long long pre, unsigned long long now);

void set_timer(unsigned int time, void (*handle)());
void delay_u(unsigned int delay_time);

static void udelay(unsigned int time){
	for(volatile unsigned int i = 0; i < time; i++)
		for(volatile int i = 0; i < 35; i++);
}

static void mdelay(unsigned int time){
	for(volatile unsigned int i = 0; i < time; i++)
		for(volatile unsigned int i = 0; i < 1000; i++)
			for(volatile int i = 0; i < 35; i++);
}
#endif
