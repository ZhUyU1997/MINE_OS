#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
#include <sys/types.h>
enum INT_NUM{
	EINT0 = 0,
	EINT1,
	EINT2,
	EINT3,
	EINT4_7,
	EINT8_23,
	INT_CAM,
	nBATT_FLT,
	INT_TICK,
	INT_WDT_AC97,
	INT_TIMER0,
	INT_TIMER1,
	INT_TIMER2,
	INT_TIMER3,
	INT_TIMER4,
	INT_UART2,
	INT_LCD,
	INT_DMA0,
	INT_DMA1,
	INT_DMA2,
	INT_DMA3,
	INT_SDI,
	INT_SPI0,
	INT_UART1,
	INT_NFCON,
	INT_USBD,
	INT_USBH,
	INT_IIC,
	INT_UART0,
	INT_SPI1,
	INT_RTC,
	INT_ADC
};
enum INT_SUB_NUM{
	INT_RXD0,
	INT_TXD0,
	INT_ERR0,
	INT_RXD1,
	INT_TXD1,
	INT_ERR1,
	INT_RXD2,
	INT_TXD2,
	INT_ERR2,
	INT_TC,
	INT_ADC_S,
	INT_CAM_C,
	INT_CAM_P,
	INT_WDT,
	INT_AC97,
};

enum EINT_NUM{
	EINT4 = 4,
	EINT5,
	EINT6,
	EINT7,
	EINT8,
	EINT9,
	EINT10,
	EINT11,
	EINT12,
	EINT13,
	EINT14,
	EINT15,
	EINT16,
	EINT17,
	EINT18,
	EINT19,
	EINT20,
	EINT21,
	EINT22,
	EINT23,
};
void enable_irq(void);
void disable_irq(void);
void set_subint(enum INT_SUB_NUM num);
void clr_subint(enum INT_SUB_NUM num);
void save_subsrcpnd(U32 reg);
U32 get_subsrcpnd();
void request_irq(enum INT_NUM num,int (*hander)(void));
void free_irq(enum INT_NUM offset);
#endif
