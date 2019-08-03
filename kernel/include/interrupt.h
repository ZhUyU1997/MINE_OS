#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>
#include <irqs.h>

typedef struct hw_int_type {
	void (*enable)(unsigned long irq);
	void (*disable)(unsigned long irq);

	void (*mask)(unsigned long irq);
	void (*unmask)(unsigned long irq);

	unsigned long(*install)(unsigned long irq, void * arg);
	void (*uninstall)(unsigned long irq);

	void (*ack)(unsigned long irq);
} hw_int_controller;

struct irq_desc;

typedef	void (*irq_flow_handler_t)(unsigned int irq, struct irq_desc *desc);
//typedef void (*irq_handler_t)(unsigned long nr, unsigned long parameter, struct pt_regs * regs);
typedef void (*irq_handler_t)(unsigned long nr, unsigned long parameter);

typedef struct irq_desc {
	hw_int_controller * controller;
	char * irq_name;
	unsigned long parameter;
	irq_flow_handler_t	handle_irq;
	irq_handler_t handler;
	unsigned long flags;
} irq_desc_T;


extern struct irq_desc interrupt_desc[NR_IRQS];

int register_irq(unsigned long irq,
				 void * arg,
				 void (*handler)(unsigned long nr, unsigned long parameter),
				 unsigned long parameter,
				 hw_int_controller * controller,
				 char * irq_name);

int unregister_irq(unsigned long irq);

void enable_irq(void);
void disable_irq(void);
void request_irq(int irq, irq_handler_t hander);
void free_irq(int irq);

#endif
