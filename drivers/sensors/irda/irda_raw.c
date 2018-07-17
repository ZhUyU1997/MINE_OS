#include <s3c24xx.h>
#include <interrupt.h>
#include "irda_raw.h"

/* IRDA引脚 : EINT1/GPF1 */

/*
 * 配置GPIO, 注册中断
 * 在中断处理函数里:
      记录中断发生的时间,
      跟上次中断的时间比较, 计算出脉冲宽度
      读取引脚极性
      把数据放入环型缓冲区
 */

/* 先实现GPIO的基本操作 */
static void irda_data_cfg_as_eint(void) {
	/* 配置为中断引脚 */
	GPFCON &= ~(3 << 2);
	GPFCON |= (2 << 2);

	/* 设置中断触发方式: 双边沿触发 */
	EXTINT0 |= (7 << 4); /* eint1 */

}

static int irda_data_get(void) {
	if (GPFDAT & (1 << 1))
		return 1;
	else
		return 0;
}

void irda_irq(int irq) {
	/* 在中断处理函数里:
	  记录中断发生的时间,
	  跟上次中断的时间比较, 计算出脉冲宽度
	  读取引脚极性
	  把数据放入环型缓冲区
	*/
	static unsigned long long g_last_time = 0;
	irda_raw_event event;
	unsigned long long cur = get_system_time_us();

	event.duration = delta_time_us(g_last_time, cur);
	event.pol      = !irda_data_get();
	ir_event_put(&event);
	g_last_time = cur;
}


/* 注册中断 */
void irda_init(void) {
	irda_data_cfg_as_eint();
	request_irq(EINT1, irda_irq);
}
void irda_exit(void) {
	//TODO
	free_irq(EINT1);
}
void irda_raw_test(void) {
	irda_raw_event event;
	unsigned long long pre = 0, cur;

	irda_init();

	while (!serial_getc_async()) {
		if (0 == ir_event_get(&event)) {
			cur = get_system_time_us();
			if (delta_time_us(pre, cur) > 1000000)
				printf("\n");
			pre = cur;
			printf("%s:%d us\n", event.pol ? "1" : "0", event.duration);
		}
	}
	irda_exit();
}

