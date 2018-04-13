#ifndef __TIMER_H__
#define __TIMER_H__

void timer_init();
void init_tick(int time, void (*handle)());
void tick_irq_hander();
int get_sys_tick();
void set_timer(int time, void (*handle)());
void udelay(int delay_time);
#endif