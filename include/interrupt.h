#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
enum INT_NUM{
	EINT0=0,
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
void EINT_Handle();
void enable_irq(void);
void disable_irq(void);
void INTMSK_set(enum INT_NUM num);
void INTMSK_clr(enum INT_NUM num);
void INTSUBMSK_set(enum INT_SUB_NUM num);
void INTSUBMSK_clr(enum INT_SUB_NUM num);
void set_irq_handler(int offset,int (*hander)(void));
#endif