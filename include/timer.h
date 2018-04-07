#ifndef __TIMER_H__
#define __TIMER_H__
void init_tick(int time, void (*handle)());
void delay_irq_hander();
int get_sys_tick();
void set_timer(int time, void (*handle)());
#endif