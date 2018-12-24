#include <assert.h>
#include <s3c24xx.h>
#include <interrupt.h>
#include <timer.h>
#include <lib.h>
#include <softirq.h>
#include <schedule.h>
#include <smp.h>

static unsigned int delta_time = 0;

static void tick_irq_hander(unsigned long nr, unsigned long parameter) {
	jiffies++;
	if ((container_of(list_next(&timer_list_head.list), struct timer_list, list)->expire_jiffies <= jiffies))
		set_softirq_status(TIMER_SIRQ);
	switch(current->priority)
	{
		case 0:
		case 1:
			task_schedule[SMP_cpu_id()].CPU_exec_task_jiffies--;
			current->vrun_time += 1;
			break;
		case 2:
		default:
			task_schedule[SMP_cpu_id()].CPU_exec_task_jiffies -= 2;
			current->vrun_time += 2;
			break;
	}

	if(task_schedule[SMP_cpu_id()].CPU_exec_task_jiffies <= 0)
		current->flags |= NEED_SCHEDULE;
}

void init_tick(unsigned int time) {
	assert(time <= 0xffff);
	//TCON:定时器控制寄存器
	TCON &= ~(1 << 20);		//关闭
	//free_irq(IRQ_TIMER4);
	TCON &= ~(7 << 20);	//清空20~21位
	TCON |= (1 << 22);		//定时器4间隙模式/自动重载
	//TCONB4:定时器4计数缓冲寄存器

	TCNTB4 = delta_time = time;
	TCON |= (1 << 21);		//定时器4手动更新TCNTB4
	TCON &= ~(1 << 21);		//定时器4取消手动更新
	request_irq(IRQ_TIMER4, tick_irq_hander);
	TCON |= (1 << 20);		//启动
}

int get_sys_tick() {
	return jiffies;
}

unsigned long long get_system_time_us(void) {
	return delta_time * jiffies + (delta_time - TCNTO4);
}

unsigned long long delta_time_us(unsigned long long pre, unsigned long long now) {
	return (now - pre);
}
/*
static void (*timer_handle)() = 0;

static void timer_handler(unsigned long nr, unsigned long parameter) {
	TCON &= ~(1 << 8); //关闭
	free_irq(IRQ_TIMER1);
	if (timer_handle)
		timer_handle();
}


void set_timer(unsigned int time, void (*handle)()) {
	assert((100 * time <= 0xffff) && handle);
	if (!handle)
		return;
	TCON &= ~(1 << 8); //关闭
	//free_irq(IRQ_TIMER1);
	//TCON:定时器控制寄存器
	TCFG1 &= ~(15 << 4);
	TCFG1 |= 1 << 4;
	TCON &= ~(15 << 8);	//清空8~11位
	TCON &= ~(1 << 11);		//定时器1单稳态

	//TCONB1:定时器1计数缓冲寄存器
	TCNTB1 = 100 * time;
	TCMPB1 = 0;
	TCON |= (1 << 9);		//定时器1手动更新TCNTB1和TCMPB1
	TCON &= ~(1 << 9);		//定时器1取消手动更新
	timer_handle = handle;
	request_irq(IRQ_TIMER1, timer_handler);
	TCON |= (1 << 8);		//启动
}

void close_timer() {
	TCON &= ~(1 << 8); //关闭
	//free_irq(IRQ_TIMER1);
}
*/


static volatile int delay_end = 0;

static void delay_irq_hander(unsigned long nr, unsigned long parameter) {
	TCON &= ~(1 << 12); //定时器关闭
	free_irq(IRQ_TIMER2);
	delay_end = 1;
}
void delay_u(unsigned int delay_time) {
	assert(delay_time <= 0xffff);
	TCON &= ~(1 << 12); //关闭
	//free_irq(IRQ_TIMER2);

	if (delay_time > 0xffff)
		delay_time = 0xffff;


	//TCON:定时器控制寄存器
	TCON &= ~(0x0f << 12);	//清空12~11位
	TCON &= ~(1 << 15);		//定时器2单稳态

	//TCONB2:定时器2计数缓冲寄存器
	TCNTB2 = delay_time;
	TCMPB3 = 0;
	TCON |= (1 << 13);		//定时器2手动更新TCNTB2和TCMPB2
	TCON &= ~(1 << 13);		//定时器2取消手动更新
#if 1
	request_irq(IRQ_TIMER2, delay_irq_hander);
	delay_end = 0;
	TCON |= (1 << 12);		//启动
	while (!delay_end);
#else
	//之所以提供无中断版本，是防止中断屏蔽等情况，方便测试
	TCON |= (1 << 12);		//启动
	//有时启动后TCNTO2无法立即更新，所以1us延迟
	udelay(1);
	while (TCNTO2);
#endif
	//free_irq(IRQ_TIMER2);
}

/*
 * 先写TCNTBn,TCMPBn，在启动手动更新，而不是启动手动更新，再写寄存器
 */

void s3c2440_timer_init() {
	TCFG0 = 0;
	TCFG1 = 0;
	TCON  = 0;
	TCFG0 |= (124);		//定时器 0，1 的预分频值
	TCFG0 |= (24 << 8);	//定时器 2，3 和 4 的预分频值
}
