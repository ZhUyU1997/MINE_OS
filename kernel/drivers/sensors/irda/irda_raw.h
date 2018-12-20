#ifndef _IRDA_RAW_H
#define _IRDA_RAW_H

typedef struct irda_raw_event {
	int pol; /* 极性 */
	int duration;  /* 脉冲宽度, us */
} irda_raw_event, *p_irda_raw_event;

/* 注册中断 */
void irda_init(void);
void irda_exit(void);

#endif /* _IRDA_RAW_H */

