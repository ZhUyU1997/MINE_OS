#include <interrupt.h>
#include <assert.h>
#include <irqs.h>
#include <kernel.h>
#include <__ffs.h>
#include "s3c24xx.h"


#define irqdbf2(...)
#define EXTINT_OFF (IRQ_EINT4 - 4)

#define INTMSK_UART0		(1UL << (IRQ_UART0 - IRQ_EINT0))
#define INTMSK_UART1		(1UL << (IRQ_UART1 - IRQ_EINT0))
#define INTMSK_UART2		(1UL << (IRQ_UART2 - IRQ_EINT0))
#define INTMSK_ADCPARENT	(1UL << (IRQ_ADCPARENT - IRQ_EINT0))
#define INTMSK_WDT			(1UL << (IRQ_WDT - IRQ_EINT0))
#define INTMSK_CAM			(1UL << (IRQ_CAM - IRQ_EINT0))


static void s3c_irq_mask(int irq) {
	unsigned long mask;

	mask = INTMSK;
	mask |= 1UL << (irq - IRQ_EINT0);
	INTMSK = mask;
}
static void s3c_irq_unmask(int irq) {
	unsigned long mask;

	if (irq != IRQ_TIMER4 && irq != IRQ_EINT8t23)
		irqdbf2("s3c_irq_unmask %d\n", irq);

	mask = INTMSK;
	mask &= ~(1UL << (irq - IRQ_EINT0));
	INTMSK = mask;
}
static inline void s3c_irq_ack(int irq) {
	unsigned long bitval = 1UL << (irq - IRQ_EINT0);

	SRCPND = bitval;
	INTPND = bitval;
}
static void s3c_irqext_mask(int irq) {
	unsigned int irqno = irq - EXTINT_OFF;
	unsigned long mask;

	mask = EINTMASK;
	mask |= (1UL << irqno);
	EINTMASK = mask;
	
	if (irq <= IRQ_EINT7) {
		if ((mask & 0xf0) == 0xf0)
			s3c_irq_mask(IRQ_EINT4t7);
	} else {
		if ((mask >> 8) == 0xffff)
			s3c_irq_mask(IRQ_EINT8t23);
	}
}
static void s3c_irqext_unmask(int irq) {
	unsigned int irqno = irq - EXTINT_OFF;
	unsigned long mask;

	mask = EINTMASK;
	mask &= ~(1UL << irqno);
	EINTMASK = mask;

	if (irq <= IRQ_EINT7) {
		if ((mask & 0xf0) != 0xf0)
			s3c_irq_unmask(IRQ_EINT4t7);
	} else {
		if ((mask >> 8) != 0xffff)
			s3c_irq_unmask(IRQ_EINT8t23);
	}
}
static inline void s3c_irqext_ack(int irq) {
	unsigned long req;
	unsigned long bit;
	unsigned long mask;

	bit = 1UL << (irq - EXTINT_OFF);
	mask = EINTMASK;
	EINTPEND = bit;

	req = EINTPEND;
	req &= ~mask;

	/* not sure if we should be acking the parent irq... */

	if (irq <= IRQ_EINT7) {
		if ((req & 0xf0) == 0)
			s3c_irq_ack(IRQ_EINT4t7);
	} else {
		if ((req >> 8) == 0)
			s3c_irq_ack(IRQ_EINT8t23);
	}
}
static inline void s3c_irqsub_mask(unsigned int irq) {
	unsigned long mask;
	unsigned long submask;
	unsigned int parentbit = 0;
	int subcheck = 0;

	submask = INTSUBMSK;
	mask = INTMSK;

	submask |= (1UL << (irq - IRQ_S3CUART_RX0));

	switch (irq) {
		case IRQ_S3CUART_RX0 ... IRQ_S3CUART_ERR0:
			subcheck = 7;
			parentbit = INTMSK_UART0;
			break;
		case IRQ_S3CUART_RX1 ... IRQ_S3CUART_ERR1:
			subcheck = 7 << 3;
			parentbit = INTMSK_UART1;
			break;
		case IRQ_S3CUART_RX2 ... IRQ_S3CUART_ERR2:
			subcheck = 7 << 6;
			parentbit = INTMSK_UART2;
			break;
		case IRQ_TC ... IRQ_ADC:
			subcheck = 3 << 9;
			parentbit = INTMSK_ADCPARENT;
			break;
		case IRQ_S3C2440_CAM_C ... IRQ_S3C2440_CAM_P:
			subcheck = 3 << 11;
			parentbit = INTMSK_CAM;
			break;
		case IRQ_S3C2440_WDT ... IRQ_S3C2440_AC97:
			subcheck = 3 << 13;
			parentbit = INTMSK_WDT;
			break;
		default:
			assert(0);
	}

	/* check to see if we need to mask the parent IRQ */

	if ((submask  & subcheck) == subcheck)
		INTMSK = mask | parentbit;

	/* write back masks */
	INTSUBMSK = submask;

}
static inline void s3c_irqsub_unmask(unsigned int irq) {
	unsigned long mask;
	unsigned long submask;
	unsigned int parentbit = 0;

	submask = INTSUBMSK;
	mask = INTMSK;

	switch (irq) {
		case IRQ_S3CUART_RX0 ... IRQ_S3CUART_ERR0:
			parentbit = INTMSK_UART0;
			break;
		case IRQ_S3CUART_RX1 ... IRQ_S3CUART_ERR1:
			parentbit = INTMSK_UART1;
			break;
		case IRQ_S3CUART_RX2 ... IRQ_S3CUART_ERR2:
			parentbit = INTMSK_UART2;
			break;
		case IRQ_TC ... IRQ_ADC:
			parentbit = INTMSK_ADCPARENT;
			break;
		case IRQ_S3C2440_CAM_C ... IRQ_S3C2440_CAM_P:
			parentbit = INTMSK_CAM;
			break;
		case IRQ_S3C2440_WDT ... IRQ_S3C2440_AC97:
			parentbit = INTMSK_WDT;
			break;
		default:
			assert(0);
	}

	submask &= ~(1UL << (irq - IRQ_S3CUART_RX0));
	mask &= ~parentbit;

	/* write back masks */
	INTSUBMSK = submask;
	INTMSK = mask;
}
static inline void s3c_irqsub_ack(unsigned int irq) {
	unsigned int bit = 1UL << (irq - IRQ_S3CUART_RX0);
	unsigned int parentmask = 0;

	SUBSRCPND = bit;

	/* only ack parent if we've got all the irqs (seems we must
	 * ack, all and hope that the irq system retriggers ok when
	 * the interrupt goes off again)
	 */

	switch (irq) {
		case IRQ_S3CUART_RX0 ... IRQ_S3CUART_ERR0:
			parentmask = INTMSK_UART0;
			break;
		case IRQ_S3CUART_RX1 ... IRQ_S3CUART_ERR1:
			parentmask = INTMSK_UART1;
			break;
		case IRQ_S3CUART_RX2 ... IRQ_S3CUART_ERR2:
			parentmask = INTMSK_UART2;
			break;
		case IRQ_TC ... IRQ_ADC:
			parentmask = INTMSK_ADCPARENT;
			break;
		case IRQ_S3C2440_CAM_C ... IRQ_S3C2440_CAM_P:
			parentmask = INTMSK_CAM;
			break;
		case IRQ_S3C2440_WDT ... IRQ_S3C2440_AC97:
			parentmask = INTMSK_WDT;
			break;
		default:
			assert(0);
	}

	if (1) {
		SRCPND = parentmask;
		INTPND = parentmask;
	}
}

void irq_set_handler(int irq, irq_flow_handler_t handler) {
	assert((irq >= S3C2440_CPUIRQ_OFFSET) && (irq < NR_IRQS));
	if (handler)
		interrupt_desc[irq].handle_irq = handler;
	else
		assert(0);
}

void irq_set_controller(int irq, hw_int_controller *controller) {
	assert((irq >= S3C2440_CPUIRQ_OFFSET) && (irq < NR_IRQS));
	if (controller)
		interrupt_desc[irq].controller = controller;
	else
		assert(0);
}

void generic_handle_irq_desc(int irq, struct irq_desc *desc) {
	if (desc)
		desc->handle_irq(irq, desc);
	else
		assert(0);
}

struct irq_desc *irq_to_desc(unsigned int irq) {
	return (irq < NR_IRQS) ? interrupt_desc + irq : NULL;
}

int generic_handle_irq(unsigned int irq) {
	struct irq_desc *desc = irq_to_desc(irq);

	if (!desc)
		return -1;
	generic_handle_irq_desc(irq, desc);
	return 0;
}

/* irq demux for adc */
static void s3c_irq_demux_adc(unsigned int irq, struct irq_desc *desc) {
	unsigned int subsrc, submsk;
	unsigned int offset = 9;

	/* read the current pending interrupts, and the mask
	 * for what it is available */

	subsrc = SUBSRCPND;
	submsk = INTSUBMSK;

	subsrc &= ~submsk;
	subsrc >>= offset;
	subsrc &= 3;

	if (subsrc != 0) {
		if (subsrc & 1) {
			generic_handle_irq(IRQ_TC);
		}
		if (subsrc & 2) {
			generic_handle_irq(IRQ_ADC);
		}
	}
}

static void s3c_irq_demux_uart(unsigned int start) {
	unsigned int subsrc, submsk;
	unsigned int offset = start - IRQ_S3CUART_RX0;

	/* read the current pending interrupts, and the mask
	 * for what it is available */

	subsrc = SUBSRCPND;
	submsk = INTSUBMSK;

	irqdbf2("s3c_irq_demux_uart: start=%d (%d), subsrc=0x%08x,0x%08x\n",
		   start, offset, subsrc, submsk);

	subsrc &= ~submsk;
	subsrc >>= offset;
	subsrc &= 7;

	if (subsrc != 0) {
		if (subsrc & 1)
			generic_handle_irq(start);

		if (subsrc & 2)
			generic_handle_irq(start + 1);

		if (subsrc & 4)
			generic_handle_irq(start + 2);
	}
}

/* uart demux entry points */

static void
s3c_irq_demux_uart0(unsigned int irq, struct irq_desc *desc) {
	irq = irq;
	s3c_irq_demux_uart(IRQ_S3CUART_RX0);
}

static void
s3c_irq_demux_uart1(unsigned int irq, struct irq_desc *desc) {
	irq = irq;
	s3c_irq_demux_uart(IRQ_S3CUART_RX1);
}

static void s3c_irq_demux_uart2(unsigned int irq, struct irq_desc *desc) {
	irq = irq;
	s3c_irq_demux_uart(IRQ_S3CUART_RX2);
}

static void s3c_irq_demux_extint8(unsigned int irq, struct irq_desc *desc) {
	unsigned long eintpnd = EINTPEND;
	unsigned long eintmsk = EINTMASK;

	eintpnd &= ~eintmsk;
	eintpnd &= ~0xff;	/* ignore lower irqs */

	/* we may as well handle all the pending IRQs here */

	while (eintpnd) {
		irq = __ffs(eintpnd);
		eintpnd &= ~(1 << irq);

		irq += (IRQ_EINT4 - 4);
		generic_handle_irq(irq);
	}
}

static void s3c_irq_demux_extint4t7(unsigned int irq, struct irq_desc *desc) {
	unsigned long eintpnd = EINTPEND;
	unsigned long eintmsk = EINTMASK;

	eintpnd &= ~eintmsk;
	eintpnd &= 0xff;	/* only lower irqs */

	/* we may as well handle all the pending IRQs here */

	while (eintpnd) {
		irq = __ffs(eintpnd);
		eintpnd &= ~(1 << irq);

		irq += (IRQ_EINT4 - 4);

		generic_handle_irq(irq);
	}
}

/* WDT/AC97 */

static void s3c_irq_demux_wdtac97(unsigned int irq, struct irq_desc *desc) {
	unsigned int subsrc, submsk;

	/* read the current pending interrupts, and the mask
	 * for what it is available */

	subsrc = SUBSRCPND;
	submsk = INTSUBMSK;

	subsrc &= ~submsk;
	subsrc >>= 13;
	subsrc &= 3;

	if (subsrc != 0) {
		if (subsrc & 1) {
			generic_handle_irq(IRQ_S3C2440_WDT);
		}
		if (subsrc & 2) {
			generic_handle_irq(IRQ_S3C2440_AC97);
		}
	}
}

/* camera irq */

static void s3c_irq_demux_cam(unsigned int irq, struct irq_desc *desc) {
	unsigned int subsrc, submsk;

	/* read the current pending interrupts, and the mask
	 * for what it is available */

	subsrc = SUBSRCPND;
	submsk = INTSUBMSK;

	subsrc &= ~submsk;
	subsrc >>= 11;
	subsrc &= 3;

	if (subsrc != 0) {
		if (subsrc & 1) {
			generic_handle_irq(IRQ_S3C2440_CAM_C);
		}
		if (subsrc & 2) {
			generic_handle_irq(IRQ_S3C2440_CAM_P);
		}
	}
}


void handle_irq(unsigned int irq, struct irq_desc *desc) {
	if (desc->handler)
		desc->handler(irq, desc->parameter);
	if (desc->controller && desc->controller->ack)
		desc->controller->ack(irq);
}


hw_int_controller con_irq = {
	.ack = s3c_irq_ack,
	.mask = s3c_irq_mask,
	.unmask = s3c_irq_unmask,
};
hw_int_controller con_irqsub = {
	.ack = s3c_irqsub_ack,
	.mask = s3c_irqsub_mask,
	.unmask = s3c_irqsub_unmask,
};
hw_int_controller con_irqext = {
	.ack = s3c_irqext_ack,
	.mask = s3c_irqext_mask,
	.unmask = s3c_irqext_unmask,
};

void irq_init(void) {
	EINTMASK = 0xfffff0;
	INTMOD = 0x0;	      //所有中断都设为IRQ模式
	INTMSK = BIT_ALLMSK;  //先屏蔽所有中断
	INTSUBMSK = 0xffff;

	EINTPEND = 0xfffff0;
	SRCPND = 0xffffffff;
	INTPND = 0xffffffff;
	SUBSRCPND = 0xffffffff;

	for (int irqno = IRQ_EINT0; irqno < NR_IRQS; irqno++) {
		irq_set_handler(irqno, handle_irq);
	}

	for (int irqno = IRQ_EINT0; irqno <= IRQ_ADCPARENT; irqno++) {
		irq_set_controller(irqno, &con_irq);
	}

	for (int irqno = IRQ_EINT4; irqno <= IRQ_EINT23; irqno++) {
		irq_set_controller(irqno, &con_irqext);
	}

	for (int irqno = IRQ_S3CUART_RX0; irqno <= IRQ_S3C2440_AC97; irqno++) {
		irq_set_controller(irqno, &con_irqext);
	}

	irq_set_handler(IRQ_EINT4t7, s3c_irq_demux_extint4t7);
	irq_set_handler(IRQ_EINT8t23, s3c_irq_demux_extint8);
	irq_set_handler(IRQ_UART0, s3c_irq_demux_uart0);
	irq_set_handler(IRQ_UART1, s3c_irq_demux_uart1);
	irq_set_handler(IRQ_UART2, s3c_irq_demux_uart2);
	irq_set_handler(IRQ_ADCPARENT, s3c_irq_demux_adc);
	irq_set_handler(IRQ_CAM, s3c_irq_demux_cam);
	irq_set_handler(IRQ_WDT, s3c_irq_demux_wdtac97);
}
