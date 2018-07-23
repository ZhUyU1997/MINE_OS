#ifndef __TIMER_H__
#define __TIMER_H__
#include <lib.h>
extern unsigned long long volatile jiffies;

struct timer_list {
	struct List list;
	unsigned long expire_jiffies;
	void (* func)(void * data);
	void *data;
};

extern struct timer_list timer_list_head;

void init_timer(struct timer_list * timer, void (* func)(void * data), void *data, unsigned long expire_jiffies);
void add_timer(struct timer_list * timer);
void del_timer(struct timer_list * timer);
void timer_init();
void do_timer();

void init_tick(unsigned int time);
int get_sys_tick();
unsigned long long get_system_time_us(void);
unsigned long long delta_time_us(unsigned long long pre, unsigned long long now);

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
